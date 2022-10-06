#! /bin/bash
FILE1="FILELIST1"
SDIR="/afs/cern.ch/user/q/qili/workspace/WWWW/Ref/CMSSW_11_2_4/src/MYTEST_ZG_mini/"
WDIR="/tmp/qili/work_miniv1"
ODIR="/afs/cern.ch/user/q/qili/workspace/WWWW/Ref/CMSSW_11_2_4/src/MYTEST_ZG_mini/out/"
j=0
for FILENAME in $( cat $FILE1 ); do
  echo ${FILENAME}
  let "j=j+1"
  echo $j
cat > subscript_$j.sh <<EOF 
#!/bin/bash
pushd ${CMSSW_BASE}/src/
eval \`scram runtime -sh\`
source Rivet/rivetSetup.sh
export X509_USER_PROXY=/afs/cern.ch/user/q/qili/x509up_u25950
popd
mkdir -p ${WDIR}_$j
cd ${WDIR}_$j
cp ${SDIR}/Rivet-on-MiniAOD.py .
cmsRun Rivet-on-MiniAOD.py  inputFiles="${FILENAME}"
cp *yoda ${ODIR}/$j.yoda
cd ${ODIR}
rm -r -f ${WDIR}_$j
EOF
cat > condor_$j.sub <<EOF 
universe              = vanilla
executable            = subscript_$j.sh
output                = hello_$j.out
error                 = hello_$j.err
log                   = hello_${cluster}_${process}.log
+JobFlavour           = "workday"
queue
EOF
  chmod 755 subscript_$j.sh
  chmod 755 condor_$j.sub
  condor_submit condor_$j.sub
done
