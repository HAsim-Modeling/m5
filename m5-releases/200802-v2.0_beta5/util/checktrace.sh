

for trace in */ethertrace
do
    bad=`tethereal -r $trace -q  -z "io,stat,100,tcp.analysis.retransmission||tcp.analysis.fast_retransmission||tcp.analysis.out_of_order||tcp.analysis.lost_segment||tcp.analysis.ack_lost_segment||tcp.analysis.window_full||tcp.analysis.duplicate_ack||tcp.analysis.duplicate_ack_num||tcp.analysis.duplicate_ack_frame"  | grep 000.000 | awk '{print $2}'`
    name=`dirname $trace`

    if [ "$bad" != "0" ] 
    then
       echo "Run $name had problems."
    fi
done
