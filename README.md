
# State Instrument(SVRM output Symbolizer) & SVRM-Learner (SVRM modeler)& SVRM-Composer (Corpus Generator)
this is the repo of SVRM-Learner (src) \ SVRM-Composer \ StateInstrument

To reproduce, you can use dokcerfile in docker-examples

note1: the origin name of SVRM-leaner is SVCS model. For better understanding in paper, we use SVRM instead of SVCS (CS means control segment)

note2: we only provide works we make in paper. Reperesentative Message recognized by Netpiler/manual selected and state variable recognized by our SVF tools is not include.


## State-Instrument Usage
### LLVM and clang install and version
recommend using LLVM-10 ,other verision LLVM not tested.
### setting environment variables
```STATERT``` should be state-instrument repo's path

```SVCONFIG```should be state variable json information file ```SVInfos.json```'s path

### building with LLVM PASS and json configuration
example of building lightftp

```
ENV SVCONFIG="${WORKDIR}/SVConfig"
ENV STATERT="${WORKDIR}/state-instrument"

ENV CC=clang-10
ENV CFLAGS='-fno-discard-value-names -g -O0 -Xclang -load -Xclang /home/ubuntu/state-instrument/SVInstrument_Pass.so' 
ENV SVINFOS_PATH=/home/ubuntu/SVConfig
```

## SVRM-Learner Usage
### Prebuild SVRM-Learner
install graphviz maven openjdk-11-jdk 
### SVRM-Learner Build
Build a self-contained jar file using the following command:
`mvn package shade:shade`
### SVRM-Learner Usage
`java -jar SVCSLearner-0.0.1-SNAPSHOT.jar <configuration file>`

example configuration can be found in docker-examples

## SVRM-composer Usage
```
python3 SVRM-Composer/FSM2Seed.py -a <SVRM model dot file> -m <Representative Message path> -o <corpus output path>
```
an example:```python3 SVRM-Composer/FSM2Seed.py -a output_model/hypothesis_1.dot -m in-ftp -o output_corpus ```

# Experimental Data
since the origin AFLNET output data is extrmely big(7.8 G) , We only provide plotting data process by our script.

If you are insterested in the raw data, please contact me.
