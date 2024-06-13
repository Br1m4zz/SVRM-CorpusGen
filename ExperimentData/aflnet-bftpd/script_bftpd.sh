#! /bin/bash
WORKDIR=`pwd`
OUTDIR=$WORKDIR/out
SCRIPTSDIR=/home/lwfdev/MYWORK/State-Benchmark/1.experiment_data/0.analyse_scripts/aflnet/multi-test
runtime=5

source /home/lwfdev/venv/Netwx/bin/activate

rm -rf $OUTDIR
mkdir $OUTDIR

for ((runs=1; runs<=runtime; runs++))
do 
    python3 $SCRIPTSDIR/state_cov_run.py -i $WORKDIR/$runs/bftpd-out-composed/replayable-new-ipsm-paths -s SVRM -r $runs -o $OUTDIR
    python3 $SCRIPTSDIR/state_cov_run.py -i $WORKDIR/$runs/bftpd-out-origin/replayable-new-ipsm-paths -s Manual -r $runs -o $OUTDIR


    python3 $SCRIPTSDIR/TPC_cov_run.py -i $WORKDIR/$runs/bftpd-out-composed/cov_over_time.csv -s SVRM -r $runs -o $OUTDIR
    python3 $SCRIPTSDIR/TPC_cov_run.py -i $WORKDIR/$runs/bftpd-out-origin/cov_over_time.csv -s Manual -r $runs -o $OUTDIR

done




python3 $SCRIPTSDIR/combine_run_iters.py -i $OUTDIR
echo "generating final result"
# python3 $SCRIPTSDIR/mutation_plot_run.py  -i $OUTDIR/mutation_info.csv -o $OUTDIR/mutations.png
python3 $SCRIPTSDIR/state_cov_plot_run2_corpus_comparison.py -i $OUTDIR/state_TPC_coverage.csv -r $runtime -c 43200 -s 1000 -o $OUTDIR/out.png
python3 $SCRIPTSDIR/state_cov_plot_run2_corpus_comparison.py -i $OUTDIR/state_TPC_coverage.csv -r $runtime -c 43200 -s 1000 -o $OUTDIR/out.pdf
# python3 $SCRIPTSDIR/state_cov_plot.py $WORKDIR/state_trans.csv
# python3 $SCRIPTSDIR/mutation_plot.py $WORKDIR/mutation_info_scattered.csv
# python3 $SCRIPTSDIR/mutation_plot.py $WORKDIR/mutation_info_composed.csv
# python3 $SCRIPTSDIR/mutation_plot.py $WORKDIR/mutation_info_origin.csv

