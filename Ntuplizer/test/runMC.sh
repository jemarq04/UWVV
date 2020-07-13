#echo "Running 2016 signal MC"
#cmsRun ntuplize_cfg.py channels=zz isMC=1 eCalib=1 muCalib=1 isSync=1 year=2016

#echo "Running 2017 signal MC"
#cmsRun ntuplize_cfg.py channels=zz isMC=1 eCalib=1 muCalib=1 isSync=0 year=2017

echo "Running 2018 signal MC"
cmsRun ntuplize_cfg.py channels=zz isMC=1 eCalib=1 muCalib=1 isSync=0 year=2018 
