# Reproduceable
## build docker
 For example: 
```
cd docker-examples/LightFTP
docker build -t svrm-live555 .
```
the process will automatically: 
1. download instrumented target with state variables provided in ```docker-examples/live555/SVConfig/SVInfos.json```
2. compile target with State-Instrument
3. building SVRM-Learner

## run learner
run in docker
```
java -jar SVRM-Learner/target/SVCSLearner-0.0.1-SNAPSHOT.jar SVConfig/svcs.properties
```
the process will run SVRM-Learner with configuration 

configuration example can be found in ```docker-examples/live555/SVConfig/SVInfos.json```

## run composer
run composer

note: the python scripts rely on some packages which docker may not provided.
```
python3 SVRM-Composer/FSM2Seed.py -a <SVRM model dot file> -m <Representative Message path> -o <corpus output path>
```
an example:```python3 SVRM-Composer/FSM2Seed.py -a output_model/hypothesis_1.dot -m in-ftp -o output_corpus ```
