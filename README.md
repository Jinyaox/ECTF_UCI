# 2023 MITRE eCTF Challenge: Protected Automotive Remote Entry Device (PARED)

## Design Structure
- `car` - source code for building car devices
- `deployment` - source code for generating deployment-wide secrets
- `docker_env` - source code for creating docker build environment
- `fob` - source code for building key fob devices
- `host_tools` - source code for the host tools


To Build The Car (Please note the fob is still not working right now), please run through the following step:

1. sudo docker rm $(sudo docker ps -aq) //this removes all existing images
2. sudo docker rm image [your image id] 
3. Follow all the steps from section 3 tech specification up to build car-fob pair
  
    - You can use any command for build car fob pair but this is what I used (to run this command successfully, make sure you name your deployment as depl)

sudo python3 -m ectf_tools build.car_fob_pair --design ECTF_UCI/ --name test_car --deployment depl --car-out ECTF_UCI/car --fob-out ECTF_UCI/fob --car-name car1 --fob-name fob1 --car-id 01 --pair-pin 123456 --car-unlock-secret secret1 --car-feature1-secret f1 --car-feature2-secret f2 --car-feature3-secret f3
