#!/bin/bash
set -e
. /var/lib/jenkins/jobs/master.get_branch_repo/workspace/big-data/pack-funcs

type="n2n"

echo "Check version"
checkVersion "n2n" $type

echo "removing previous debians from jenkins"
rm /var/lib/jenkins/Automation/Automation_ISO/work/custom-iso/pool/extras/subutai-n2n*.deb
rm /var/lib/jenkins/Automation/Automation_ISO/work/custom-iso/pool/extras/subutai-tap-create*.deb

echo "Version is changed. Build new debian"
/bin/bash ./scripts/mk_deb.sh

echo "Copying new debian under pool/extras"
cp build_deb/*.deb /var/lib/jenkins/Automation/Automation_ISO/work/custom-iso/pool/extras

