import os
import sys

result = {}

def get_statics(name, recv, sent):
    sum = 0
    count = 0
    slow_count, very_slow_count, ultra_slow_count = 0, 0, 0
    for i in range(300):
        delay = recv[i] - sent[i]

        if delay >= 0:
            sum += delay
            count += 1
            if delay >= 10:
                slow_count += 1
            if delay >= 25:
                very_slow_count += 1
            if delay >= 50:
                ultra_slow_count += 1
            if sent[i] == -1 and recv[i] == -1:
                count -= 1
        elif delay >= -2:
            count += 1
        else :
            print(name + " abnormal data: no." + str(i) + " recv:" + str(recv[i]) + " sent:" + str(sent[i]) + " delay:" + str(delay))

        print(name + " data: no." + str(i) + " recv:" + str(recv[i]) + " sent:" + str(sent[i]) + " delay:" + str(delay))
    if count == 0:
        print(name + " get nothing")
    else:
        print(name + " avg:" + str(sum/count) + "ms")
        print(name + " loss rate:" + str(100-count/3) + "%\n")
        print(name + " slow rate:" + str(slow_count/10) + "%\n")
        print(name + " very slow rate:" + str(very_slow_count/10) + "%\n")
        print(name + " ultra slow rate:" + str(ultra_slow_count/10) + "%\n")

    if count == 0:
        result[name] = -1
    else :
        result[name] = sum/count

ignore = []
ignore_file = []
multi_reader = []
related_file = []
app = [[8, 9], [2, 5], [6, 7]]

for i in range(10):
    exec("topic%s_recv=[-1]*300" % (i))
    exec("topic%s_sent=[-1]*300" % (i))

if len(sys.argv) > 1:
    dir = "./result/" + sys.argv[1] + "/"
else:
    dir = "./result/xxx/"

for root, dirs, files in os.walk(dir):
    for filename in files:
        offset = 0
        if "result_unit" not in filename or filename in ignore_file:
            continue 
        if filename in related_file:
            for t in multi_reader:
                exec("%s_%s_sent=[-1]*300" % (t, filename.split('.')[0]))
                exec("%s_%s_recv=[-1]*300" % (t, filename.split('.')[0]))
        if "sd_sensor" in filename:
            topic = "topic8"
            op = "sent"
        elif "sd3" in filename:
            topic = "topic8"
            op = "recv"
        elif "sd1" in filename:
            topic = "topic9"
            op = "sent"
        elif "sd_monitor" in filename:
            topic = "topic9"
            op = "recv"
        with open(dir + filename, 'r') as file:
            for line in file:
                parts = line.strip().split()
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
                exec("%s_%s[index] = int(parts[-2]) + offset" % (topic, op))
            


for i in range(10):
    if i not in ignore:
        if "topic" + str(i) in multi_reader:
            for file in related_file:
                name = file.split('.')[0]
                exec("print('topic%s_sent missed: ' + str(topic%s_sent.count(-1)))" % (i, i))
                exec("print('topic%s_%s_recv missed: ' + str(topic%s_%s_recv.count(-1)))" % (i, name, i, name))
        else:
            exec("print('topic%s_sent missed: ' + str(topic%s_sent.count(-1)))" % (i, i))
            exec("print('topic%s_recv missed: ' + str(topic%s_recv.count(-1)))" % (i, i))
    exec("print('topic%s_sent: ' + str(topic%s_sent))" % (i, i))
    exec("print('topic%s_recv: ' + str(topic%s_recv))" % (i, i))
print("\n")

for i in range(10):
    if i not in ignore:
        if "topic" + str(i) in multi_reader:
            for file in related_file:
                name = file.split('.')[0]
                exec("get_statics('t%s', topic%s_%s_recv, topic%s_sent)" % (str(i) + "_" + name, i, name, i))
        else:
            exec("get_statics('t%s', topic%s_recv, topic%s_sent)" % (i, i, i))

i=0
while i < len(app):
    print(i)
    exec("get_statics('app%s', topic%s_recv, topic%s_sent)" % (i, app[i][1], app[i][0]))
    i += 1

def find_indices(lst, target):
    indices = []
    for i, value in enumerate(lst):
        if value == target:
            indices.append(i)
    return indices

#App0 utilizes forwarding from SomeIP to DDS. Given that QEMU simulated serial input requires a certain delay, relevant topics are directly summed here
#t8 and t9 are some/ip actually
result['app0'] = result['t0'] + result['t1'] + result['t8'] + result['t9'] 
print(result)

