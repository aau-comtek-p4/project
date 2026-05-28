import json
from hdrh.histogram import HdrHistogram 
import subprocess
min_remaining = 9999999999
total_remaining=0 
tick_count =0 
max_suspeded_time = 0
total_suspend_time = 0
suspend_count = 0
coroutine_started = 0
coroutine_finished = 0
suspend_times = []
log_name =  "log-6.txt"
back_trace_cmd = "addr2line -e ./build/node/COMTEK4_NODE -f -C"
class Trace:
    def __init__(self,trace_id,name,parent_id,duration_ns,actual_ns) -> None:
        self.trace_id = trace_id
        self.name = name
        self.parent_id = parent_id
        self.children = []
        self.duration_ns = duration_ns
        self.actual_ns = actual_ns
        pass
def build_trace(trace:Trace):
    with open(log_name) as f:
        for l in f:
            y = json.loads(l)

            if(y["reason"] != "trace_print"):
                continue

            parent_id = y["payload"]["parent_id"]
            if(parent_id !=  trace.trace_id):
                continue
            trace_id = y["payload"]["id"]
            name= y["payload"]["name"]
            duration= y["payload"]["duration_ns"]
            actual= y["payload"]["actual_ns"]
            new_trace = Trace(trace_id,name,parent_id,duration,actual)
            build_trace(new_trace)
            trace.children.append(new_trace)
def print_trace(trace:Trace,generation):
    padding = ""
    head_padding = ""
    if(generation != 0):
        padding = " "*(generation-1)*2 +" " * generation*2
        head_padding = " "*(generation-1)*2+ "-" * generation*2
        padding += "|"
    print(f"{head_padding}Trace: {trace.name}")
    print(f"{padding}Duration: {trace.duration_ns}")
    print(f"{padding}Actual: {trace.actual_ns}")
    print(f"{padding}ID: {trace.trace_id}")


    for child in trace.children:
        print_trace(child,generation + 1)
backtrace_addr = []

with open("lul.txt","w+") as b:
    pass

with open(log_name) as f:
    last_second = 0

    for l in f:
        y = json.loads(l)

        if(y["reason"] == "metric" or y["reason"] == "stat_metric"):
            if(y["reason"] == "stat_metric"):
                if(y["payload"]["stat_type"] == "dropped_log"):
                    if(y["payload"]["count"] != 0):
                        print(f"drpped log: {y["payload"]["count"]}")
            with open("lul.txt","a+") as b:
                b.write(l)
        if(y["reason"] == "backtrace"):
            backtrace_addr.append(y["payload"]["addr"])
        if(y["reason"] == "bucket_allocator_allocation"):
            if(y["payload"]["name"] == "buffer_allocator"):
                if(y["ts"] / 1000000000 > last_second ):
                    print(l[:-1])
                    last_second = (y["ts"] /1000000000 ) + 1
                elif(y["payload"]["remaining"] == 0):
                    print(l[:-1])

        if(y["reason"] == "trace_print"):
            parent_id= y["payload"]["parent_id"]
            if(parent_id == 0):
                print("parent ideon");
                trace_id = y["payload"]["id"]
                name= y["payload"]["name"]
                duration= y["payload"]["duration_ns"]
                actual= y["payload"]["actual_ns"]
                new_trace = Trace(trace_id,name,parent_id,duration,actual)
                build_trace(new_trace)
                print_trace(new_trace,0)
                print()
        if(y["reason"] == "tick_complete"):
            reamining = y["payload"]["remaining"]
            if(reamining < min_remaining):
                min_remaining = reamining
            total_remaining += reamining
            tick_count += 1
        if(y["reason"] == "coroutine_suspended"):
            suspeded_time = y["payload"]["actual_ns"]
            suspend_times.append(suspeded_time)
            total_suspend_time += suspeded_time
            suspend_count += 1
            if suspeded_time> max_suspeded_time:
                print(f"new max suspend: {y["payload"]["name"]}")
                max_suspeded_time = suspeded_time
        if(y["reason"] == "coroutine_started"):
            coroutine_started += 1
        if(y["reason"] == "coroutine_finished"):
            coroutine_finished+= 1
    

combined_backtrace = ""
for i in range(1, len(backtrace_addr)-2):
    print(backtrace_addr[i])
    combined_backtrace += " " + backtrace_addr[i]
print(combined_backtrace)
combined_cmd = f"{back_trace_cmd}{combined_backtrace}"
print(combined_cmd)
#subprocess.run(f"{back_trace_cmd}{combined_backtrace}")
    

"""
histo = HdrHistogram(1,max_suspeded_time,4) 
print(f"max suspend:{max_suspeded_time/1000000}ms")
print(f"suspend count:{suspend_count}")
print(f"avg suspend:{total_suspend_time/suspend_count/1000000}ms")
print(f"min remaining: {min_remaining/1000000}ms")
print(f"tick_count: {tick_count}")
print(f"avg remaining: {total_remaining/tick_count/1000000}ms")
suspend_times.sort()
increment = 1000
for i in range(len(suspend_times)):
    histo.record_value(suspend_times[i])

print(f"val: [{histo.get_value_at_percentile(92)}]")
print(f"coroutine started:{coroutine_started}")
print(f"coroutine finished:{coroutine_finished}")
with open("test.hgrm","+wb") as f: 
    histo.output_percentile_distribution(f,10)
   """ 

