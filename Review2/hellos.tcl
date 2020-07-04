#Define a 'recv' function for the class 'Agent/Ping'
Agent/Rsu instproc recv {from rtt} {
	$self instvar node_
	puts "node [$node_ id] received answer from \
              $from with round-trip-time $rtt ms."
}

set ns [new Simulator]

set tracefile [open out.tr w]
$ns trace-all $tracefile

set namfile [open out.nam w]
$ns namtrace-all $namfile

#procedure finish
proc finish {} {
	global ns tracefile namfile
	$ns flush-trace
	close $tracefile
	close $namfile
	exec nam out.nam &
	exit 0
}


#Create three nodes
set n0 [$ns node]
set n1 [$ns node]

#Connect the nodes with one link
$ns duplex-link $n0 $n1 1Mb 10ms DropTail
#Create two ping agents and attach them to the nodes n0 and n1
set p0 [new Agent/Rsu]
$ns attach-agent $n0 $p0
$p0 set offer_ 20
set p1 [new Agent/Rsu]
$ns attach-agent $n1 $p1
$p1 set offer_ 50
#Connect the two agents
$ns connect $p0 $p1
#Schedule events
$ns at 20.6 "$p1 send"
$ns at 21.8 "$p1 lda" 
$ns at 22.8 "$p1 sort"
#$ns at 0.9 "$p1 path"
#$ns at 0.1 "$p1 send"
#$ns at 0.6 "$p0 send"
#$ns at 0.6 "$p1 send"
$ns at 25.0 "finish"

#Run the simulation
$ns run
