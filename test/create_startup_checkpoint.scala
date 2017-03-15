import sys.process._
import java.io._
// arg0: startup group id
// arg1: end group id
//TODO: The assumption here is that each group has only one process (true for Make)

val outputfilename = "ckpt_clocks"
val pw = new PrintWriter(new FileOutputStream(new File(outputfilename), true))
for(i <- args(0).toInt to args(1).toInt) {
	println ("Processing group " + i)
	//1. first check executable name
	val parseckpt_command = "./parseckpt /replay_logdb/rec_" + i
	val parseckpt_result = parseckpt_command!!;
	if (parseckpt_result.contains ("record filename: /usr/lib/gcc/i686-linux-gnu/4.6/cc1")) {
		println ("####Executable is cc1")
		//2. run linkage tool to generate params log
		//seqtt /replay_logdb/rec_77829 -ckpt_clock 1191
		val link_command = "./seqtt  /replay_logdb/rec_" + i
		println ("####Executing " + link_command)
		val link_result = link_command!!;
		var last_header_fd = -1
		var last_header_clock = -1
		link_result.split("\n").toList.foreach (s => {
			if (s.startsWith("#PARAMS_LOG")) {
				val tmp = s.split(":")
				if (tmp(1) == "open" && tmp(2).endsWith(".h")) { 
					last_header_fd = tmp(3).toInt
				} else if (tmp(1) == "close" && tmp(2).toInt == last_header_fd) {
					last_header_clock = tmp(3).toInt
				} else {
					last_header_fd = -1
				}
			}		
			println(s)
		})
		println ("####Checkpoint clock should be " + last_header_clock)
		val ckpt_clock = last_header_clock
		//3. generate the checkpoint files
		val resume_command = "./resume /replay_logdb/rec_" + i + " --pthread /home/dozenow/omniplay/eglibc-2.15/prefix/lib --ckpt_at=" + ckpt_clock
		println ("####Excuting " + resume_command)
		val resume_result = resume_command!!;
		println (resume_result)
		//4.now generate recheck log with parseklog
		val d = new File("/replay_logdb/rec_" + i)
		assert (d.exists && d.isDirectory);
		val all_klogs = d.listFiles.filter(_.getName.startsWith("klog.id.")).toList
		all_klogs.foreach (s => {
			val parseklog_command = "./parseklog "+ s + " -s " + ckpt_clock
			println ("####Excuting " + parseklog_command)
			val parseklog_result = parseklog_command!!;
			println (parseklog_result)
		})
		//5. Now add this group to the startup db
		val add_command = "./parseckpt /replay_logdb/rec_" + i + " -a " + ckpt_clock
		println ("####Excuting " + add_command)
		add_command!!;
		pw.println (i + "," + ckpt_clock)
		pw.flush
	} else {
		println ("####Skipping...")
	}
}
pw.close
