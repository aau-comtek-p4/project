import json
import hdrh




def parse_file(file_name:str):
    count = 0
    a = open("parsed.txt","+a")
    with open(file_name,"r") as f:
        for l in f:
            y = json.loads(l)
            if y["reason"] != "debug":
                continue
            if y["payload"]["msg"][:3] != "RTT":
                continue
            val = y["payload"]["msg"][5:]
            a.write(val +"\n")
            count  = count +1
    print(f"count: {count}")
            

parse_file("esp_log-9.txt")

