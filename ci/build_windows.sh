#!/bin/bash
set -x

echo Fake error.  See what happens...
exit 1

WORKSPACE="workspace-${RANDOM}"
AGENT_IP="$(gcloud compute instances describe ggpo-ci-build-win-01 --zone=us-central1-a --format='get(networkInterfaces[0].accessConfigs.natIP)')"
SSH="ssh -o StrictHostKeyChecking=no"
tar -cvf ggpo.tar . &> /dev/null

${SSH} ponder@${AGENT_IP} "mkdir C:\\workspace\\${WORKSPACE}"
scp ggpo.tar "ponder@${AGENT_IP}:/workspace/${WORKSPACE}"
${SSH} ponder@${AGENT_IP} "cd C:\\workspace\\${WORKSPACE} && tar -xvf ggpo.tar && ci\\build_windows_agent.cmd"
${SSH} ponder@${AGENT_IP} "rmdir /q/s C:\\workspace\\${WORKSPACE}"

rm ggpo.tar
