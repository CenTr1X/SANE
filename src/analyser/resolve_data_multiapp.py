import os
import sys

delay_list = []

def get_statics(name, recv, sent, packets):
    sum = 0
    count = 0
    slow_count, very_slow_count, ultra_slow_count = 0, 0, 0
    for i in range(packets):
        delay = recv[i] - sent[i]
        delay_list.append(delay)
        if sent[i] == -1:
            continue
        if delay >= 0:
            sum += delay
            count += 1
            if delay >= 10:
                print(name + " no." + str(i) + " delay: " + str(delay) + " ms" + " recv: " + str(recv[i]) + " sent: " + str(sent[i]))
                slow_count += 1
            if delay >= 25:
                print(name + " no." + str(i) + " delay: " + str(delay) + " ms" + " recv: " + str(recv[i]) + " sent: " + str(sent[i]))
                very_slow_count += 1
            if delay >= 50:
                print(name + " normal data no." + str(i) + " delay: " + str(delay) + " ms" + " recv: " + str(recv[i]) + " sent: " + str(sent[i]))
                ultra_slow_count += 1
            else: 
                print(name + " no." + str(i) + " delay: " + str(delay) + " ms" + " recv: " + str(recv[i]) + " sent: " + str(sent[i]))
        elif delay >= -2:
            print(name + " too quick " + str(i) + " " + str(recv[i]) + " " + str(sent[i]))
            #sum += 0
            count += 1
        else :
            print(name + " abnormal data: no." + str(i) + " recv:" + str(recv[i]) + " sent:" + str(sent[i]))


    if count == 0:
        print(name + " get nothing")
    else:
        print(name + " avg:" + str(sum/count) + "ms")
        print(name + " loss rate:" + str((max_packet - count) / count * 100) + "%\n")
        print(name + " slow rate:" + str(slow_count/10) + "%\n")
        print(name + " very slow rate:" + str(very_slow_count/10) + "%\n")
        print(name + " ultra slow rate:" + str(ultra_slow_count/10) + "%\n")
    print(count)
num = 50
max_packet = num*300
topic0_sent = [-1] * max_packet
topic0_recv = [-1] * max_packet

if len(sys.argv) > 1:
    dir = "./result/" + sys.argv[1] + "/"
else:
    dir = "./result/num" + str(num) + "/"

for root, dirs, files in os.walk(dir):
    for filename in files:
        with open(dir + filename, 'r') as file:
            if "result" not in filename:
                continue
            for line in file:
                parts = line.strip().split()
                print(parts)
                if len(parts) == 6:
                    index = int(parts[1].replace("no.", "").replace(":", ""))
                elif len(parts) == 2 and "offset" in parts[0]:
                    offset = int(parts[1])
                    print(filename + " offset:" + str(offset))
                    continue
                else:
                    topic = parts[0]
                    op = parts[1]
                    if parts[1] == "recvf":
                        op = "recv"
                    index = int(parts[2].replace("no.", "").replace(":", ""))
                exec("topic0_%s[index] = int(parts[-2]) + offset" % (parts[1]))




print('topic%s_sent missed: ' + str(topic0_sent.count(-1)))
print('topic%s_recv missed: ' + str(topic0_recv.count(-1)))
print("\n")


get_statics('t0', topic0_recv, topic0_sent, max_packet)

def find_top_percentages(lst):
    lst.sort(reverse=True) 
    n = len(lst)
    index_25_percent = int(n * 0.25) - 1  
    index_10_percent = int(n * 0.1) - 1  
    top_25_percent = lst[index_25_percent] if index_25_percent >= 0 else None
    top_10_percent = lst[index_10_percent] if index_10_percent >= 0 else None
    return top_25_percent, top_10_percent

print(find_top_percentages(delay_list))