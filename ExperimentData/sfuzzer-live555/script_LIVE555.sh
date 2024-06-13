#! /bin/bash
WORKDIR=`pwd`
OUTDIR=$WORKDIR/out
SCRIPTSDIR=/home/lwfdev/MYWORK/State-Benchmark/1.experiment_data/0.analyse_scripts/sfuzzer/multi-test
runtime=5

source /home/lwfdev/venv/Netwx/bin/activate

rm -rf $OUTDIR
mkdir $OUTDIR

for ((runs=1; runs<=runtime; runs++))
do 
    python3 $SCRIPTSDIR/state_cov_run.py -i $WORKDIR/$runs/output-live555-composed -s SVRM -r $runs -o $OUTDIR
    python3 $SCRIPTSDIR/state_cov_run.py -i $WORKDIR/$runs/output-live555-origin -s Manual -r $runs -o $OUTDIR
    # python3 $SCRIPTSDIR/state_cov_run.py -i $WORKDIR/$runs/in-tls-scattered -s scattered -r $runs -o $OUTDIR

    # python3 $SCRIPTSDIR/TPC_cov_run.py -i $WORKDIR/$runs/exim-out-composed/cov_over_time.csv -s composed -r $runs -o $OUTDIR
    # python3 $SCRIPTSDIR/TPC_cov_run.py -i $WORKDIR/$runs/exim-out-origin/cov_over_time.csv -s origin -r $runs -o $OUTDIR
    # python3 $SCRIPTSDIR/TPC_cov_run.py -i $WORKDIR/$runs/exim-out-scattered/cov_over_time.csv -s scattered -r $runs -o $OUTDIR   
done




python3 $SCRIPTSDIR/combine_run.py -i $OUTDIR
echo "generating final result"
# python3 $SCRIPTSDIR/mutation_plot_run.py  -i $OUTDIR/mutation_info.csv -o $OUTDIR/mutations.png
python3 $SCRIPTSDIR/state_cov_plot_run_corpus.py -i $OUTDIR/state_TPC_coverage.csv -r $runtime -c 43200 -s 1000 -o $OUTDIR/out.png
python3 $SCRIPTSDIR/state_cov_plot_run_corpus.py -i $OUTDIR/state_TPC_coverage.csv -r $runtime -c 43200 -s 1000 -o $OUTDIR/out.pdf
# python3 $SCRIPTSDIR/state_cov_plot.py $WORKDIR/state_trans.csv
# python3 $SCRIPTSDIR/mutation_plot.py $WORKDIR/mutation_info_scattered.csv
# python3 $SCRIPTSDIR/mutation_plot.py $WORKDIR/mutation_info_composed.csv
# python3 $SCRIPTSDIR/mutation_plot.py $WORKDIR/mutation_info_origin.csv

