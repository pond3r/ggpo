/* -----------------------------------------------------------------------
 * GGPO.net (http://ggpo.net)  -  Copyright 2009 GroundStorm Studios, LLC.
 *
 * Use of this software is governed by the MIT license that can be found
 * in the LICENSE file.
 */

#include "input_queue.h"
#include "types.h"
#include "GGPOUE4.h"

#define PREVIOUS_FRAME(offset)   (((offset) == 0) ? (INPUT_QUEUE_LENGTH - 1) : ((offset) - 1))

InputQueue::InputQueue(int input_size)
{
   Init(-1, input_size);
}

InputQueue::~InputQueue()
{
}

void
InputQueue::Init(int id, int input_size)
{
   _id = id;
   _head = 0;
   _tail = 0;
   _length = 0;
   _frame_delay = 0;
   _first_frame = true;
   _last_user_added_frame = GameInput::NullFrame;
   _first_incorrect_frame = GameInput::NullFrame;
   _last_frame_requested = GameInput::NullFrame;
   _last_added_frame = GameInput::NullFrame;

   _prediction.init(GameInput::NullFrame, NULL, input_size);

   /*
    * This is safe because we know the GameInput is a proper structure (as in,
    * no virtual methods, no contained classes, etc.).
    */
   memset(_inputs, 0, sizeof _inputs);
   for (int i = 0; i < ARRAY_SIZE(_inputs); i++) {
      _inputs[i].size = input_size;
   }
}

int
InputQueue::GetLastConfirmedFrame()
{
   UE_LOG(GGPOLOG, Verbose, TEXT("InputQueue::GetLastConfirmedFrame returning last confirmed frame %d."), _last_added_frame);
   return _last_added_frame;
}

int
InputQueue::GetFirstIncorrectFrame()
{
   return _first_incorrect_frame;
}

void
InputQueue::DiscardConfirmedFrames(int frame)
{
   check(frame >= 0);

   if (_last_frame_requested != GameInput::NullFrame) {
      frame = MIN(frame, _last_frame_requested);
   }
   UE_LOG(GGPOLOG, Verbose, TEXT("InputQueue::DiscardConfirmedFrames discarding confirmed frames up to %d (last_added:%d length:%d [head:%d tail:%d])."),
	   frame, _last_added_frame, _length, _head, _tail);

   if (frame >= _last_added_frame) {
      _tail = _head;
   } else {
      int offset = frame - _inputs[_tail].frame + 1;
      
	  UE_LOG(GGPOLOG, Verbose, TEXT("InputQueue::DiscardConfirmedFrames difference of %d frames."), offset);
	  check(offset >= 0);

      _tail = (_tail + offset) % INPUT_QUEUE_LENGTH;
      _length -= offset;
   }

   UE_LOG(GGPOLOG, Verbose, TEXT("InputQueue::DiscardConfirmedFrames after discarding, new tail is %d (frame:%d)."), _tail, _inputs[_tail].frame);

   check(_length >= 0);
}

void
InputQueue::ResetPrediction(int frame)
{
	check(_first_incorrect_frame == GameInput::NullFrame || frame <= _first_incorrect_frame);

   UE_LOG(GGPOLOG, Verbose, TEXT("InputQueue::ResetPrediction resetting all prediction errors back to frame %d."), frame);

   /*
    * There's nothing really to do other than reset our prediction
    * state and the incorrect frame counter...
    */
   _prediction.frame = GameInput::NullFrame;
   _first_incorrect_frame = GameInput::NullFrame;
   _last_frame_requested = GameInput::NullFrame;
}

bool
InputQueue::GetConfirmedInput(int requested_frame, GameInput *input)
{
	check(_first_incorrect_frame == GameInput::NullFrame || requested_frame < _first_incorrect_frame);
   int offset = requested_frame % INPUT_QUEUE_LENGTH; 
   if (_inputs[offset].frame != requested_frame) {
      return false;
   }
   *input = _inputs[offset];
   return true;
}

bool
InputQueue::GetInput(int requested_frame, GameInput *input)
{
   UE_LOG(GGPOLOG, Verbose, TEXT("InputQueue::GetInput requesting input frame %d."), requested_frame);


   /*
    * No one should ever try to grab any input when we have a prediction
    * error.  Doing so means that we're just going further down the wrong
    * path.  check this to verify that it's true.
    */
   check(_first_incorrect_frame == GameInput::NullFrame);

   /*
    * Remember the last requested frame number for later.  We'll need
    * this in AddInput() to drop out of prediction mode.
    */
   _last_frame_requested = requested_frame;

   check(requested_frame >= _inputs[_tail].frame);

   if (_prediction.frame == GameInput::NullFrame) {
      /*
       * If the frame requested is in our range, fetch it out of the queue and
       * return it.
       */
      int offset = requested_frame - _inputs[_tail].frame;

      if (offset < _length) {
         offset = (offset + _tail) % INPUT_QUEUE_LENGTH;
		 check(_inputs[offset].frame == requested_frame);
         *input = _inputs[offset];

		 UE_LOG(GGPOLOG, Verbose, TEXT("InputQueue::GetInput returning confirmed frame number %d."), input->frame);
         return true;
      }

      /*
       * The requested frame isn't in the queue.  Bummer.  This means we need
       * to return a prediction frame.  Predict that the user will do the
       * same thing they did last time.
       */
      if (requested_frame == 0) {
		  UE_LOG(GGPOLOG, Verbose, TEXT("InputQueue::GetInput basing new prediction frame from nothing, you're client wants frame 0."));
         _prediction.erase();
		 
      } else if (_last_added_frame == GameInput::NullFrame) {
		  UE_LOG(GGPOLOG, Verbose, TEXT("InputQueue::GetInput basing new prediction frame from nothing, since we have no frames yet."));         _prediction.erase();
      } else {
		  UE_LOG(GGPOLOG, Verbose, TEXT("InputQueue::GetInput basing new prediction frame from previously added frame (queue entry:%d, frame:%d)."),
			  PREVIOUS_FRAME(_head), _inputs[PREVIOUS_FRAME(_head)].frame);
         _prediction = _inputs[PREVIOUS_FRAME(_head)];
      }
      _prediction.frame++;
   }

   check(_prediction.frame >= 0);

   /*
    * If we've made it this far, we must be predicting.  Go ahead and
    * forward the prediction frame contents.  Be sure to return the
    * frame number requested by the client, though.
    */
   *input = _prediction;
   input->frame = requested_frame;
   UE_LOG(GGPOLOG, Verbose, TEXT("InputQueue::GetInput returning prediction frame number %d (%d)."), 
	   input->frame, _prediction.frame);

   return false;
}

void
InputQueue::AddInput(GameInput &input)
{
   int new_frame;

   UE_LOG(GGPOLOG, Verbose, TEXT("InputQueue::AddInput adding input frame number %d to queue. Was first input? %s"), input.frame, _first_frame ? TEXT("true") : TEXT("false"));

   /*
    * These next two lines simply verify that inputs are passed in 
    * sequentially by the user, regardless of frame delay.
    */
   check(_last_user_added_frame == GameInput::NullFrame ||
          input.frame == _last_user_added_frame + 1);
   _last_user_added_frame = input.frame;

   UE_LOG(GGPOLOG, Verbose, TEXT("InputQueue::AddInput Setting _last_user_added_frame to %d "), _last_user_added_frame);

   /*
    * Move the queue head to the correct point in preparation to
    * input the frame into the queue.
    */
   new_frame = AdvanceQueueHead(input.frame);
   UE_LOG(GGPOLOG, Verbose, TEXT("InputQueue::AddInput Queue head set to %d"), new_frame);

   if (new_frame != GameInput::NullFrame) {
      AddDelayedInputToQueue(input, new_frame);
   }
   
   /*
    * Update the frame number for the input.  This will also set the
    * frame to GameInput::NullFrame for frames that get dropped (by
    * design).
    */
   input.frame = new_frame;

   UE_LOG(GGPOLOG, Verbose, TEXT("InputQueue::AddInput updated frame in struct to %d"), input.frame);
}

void
InputQueue::AddDelayedInputToQueue(GameInput &input, int frame_number)
{
   UE_LOG(GGPOLOG, Verbose, TEXT("InputQueue::AddDelayedInputToQueue adding delayed input frame number %d to queue."), frame_number);

   check(input.size == _prediction.size);

   check(_last_added_frame == GameInput::NullFrame || frame_number == _last_added_frame + 1);

   check(frame_number == 0 || _inputs[PREVIOUS_FRAME(_head)].frame == frame_number - 1);

   /*
    * Add the frame to the back of the queue
    */ 
   _inputs[_head] = input;
   _inputs[_head].frame = frame_number;
   _head = (_head + 1) % INPUT_QUEUE_LENGTH;
   _length++;
   _first_frame = false;

   _last_added_frame = frame_number;

   if (_prediction.frame != GameInput::NullFrame) {
	   check(frame_number == _prediction.frame);

      /*
       * We've been predicting...  See if the inputs we've gotten match
       * what we've been predicting.  If so, don't worry about it.  If not,
       * remember the first input which was incorrect so we can report it
       * in GetFirstIncorrectFrame()
       */
      if (_first_incorrect_frame == GameInput::NullFrame && !_prediction.equal(input, true)) {
		  UE_LOG(GGPOLOG, Error, TEXT("InputQueue::AddDelayedInputToQueue frame %d does not match prediction.  marking error."), frame_number);

         _first_incorrect_frame = frame_number;
      }

      /*
       * If this input is the same frame as the last one requested and we
       * still haven't found any mis-predicted inputs, we can dump out
       * of predition mode entirely!  Otherwise, advance the prediction frame
       * count up.
       */
      if (_prediction.frame == _last_frame_requested && _first_incorrect_frame == GameInput::NullFrame) {
		  UE_LOG(GGPOLOG, Error, TEXT("InputQueue::AddDelayedInputToQueue prediction is correct!  dumping out of prediction mode."));
         _prediction.frame = GameInput::NullFrame;
      } else {
         _prediction.frame++;
      }
   }
   check(_length <= INPUT_QUEUE_LENGTH);
}

int
InputQueue::AdvanceQueueHead(int frame)
{
   UE_LOG(GGPOLOG, Verbose, TEXT("InputQueue::AdvanceQueueHead advancing queue head to frame %d."), frame);

   int expected_frame = _first_frame ? 0 : _inputs[PREVIOUS_FRAME(_head)].frame + 1;
   frame += _frame_delay;

   if (expected_frame > frame) {
      /*
       * This can occur when the frame delay has dropped since the last
       * time we shoved a frame into the system.  In this case, there's
       * no room on the queue.  Toss it.
       */
	  UE_LOG(GGPOLOG, Verbose, TEXT("InputQueue::AdvanceQueueHead Dropping input frame %d (expected next frame to be %d)."), frame, expected_frame);

      return GameInput::NullFrame;
   }

   while (expected_frame < frame) {
      /*
       * This can occur when the frame delay has been increased since the last
       * time we shoved a frame into the system.  We need to replicate the
       * last frame in the queue several times in order to fill the space
       * left.
       */
	  UE_LOG(GGPOLOG, Verbose, TEXT("InputQueue::AdvanceQueueHead Adding padding frame %d to account for change in frame delay."), expected_frame);

      GameInput &last_frame = _inputs[PREVIOUS_FRAME(_head)];     
      AddDelayedInputToQueue(last_frame, expected_frame);
      expected_frame++;
   }

   check(frame == 0 || frame == _inputs[PREVIOUS_FRAME(_head)].frame + 1);
   return frame;
}
