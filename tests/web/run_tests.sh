make
source ../third-party/emsdk_portable_repo/emsdk_portable/emsdk_env.sh
for filename in *.js; 
do 
    node $filename; 
    if [ $$? -gt 0 ]; 
    then 
	exit 1; 
    fi; 
done
