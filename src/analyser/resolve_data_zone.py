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
                #print(name + " no." + str(i) + " delay: " + str(delay) + " ms" + " recv: " + str(recv[i]) + " sent: " + str(sent[i]))
                slow_count += 1
            if delay >= 25:
                #print(name + " no." + str(i) + " delay: " + str(delay) + " ms" + " recv: " + str(recv[i]) + " sent: " + str(sent[i]))
                very_slow_count += 1
            if delay >= 50:
                #print(name + " no." + str(i) + " delay: " + str(delay) + " ms" + " recv: " + str(recv[i]) + " sent: " + str(sent[i]))
                ultra_slow_count += 1
        elif delay >= -2:
            #print(name + " too quick " + str(i) + " " + str(recv[i]) + " " + str(sent[i]))
            #sum += 0
            count += 1
        #else :
            #print(name + " abnormal data: no." + str(i) + " recv:" + str(recv[i]) + " sent:" + str(sent[i]))
        print(name + " data: no." + str(i) + " recv:" + str(recv[i]) + " sent:" + str(sent[i]) + " delay:" + str(delay))

    if count == 0:
        print(name + " get nothing")
    else:
        print(name + " avg:" + str(sum/count) + "ms")
        print(name + " loss rate:" + str(100-count/3) + "%\n")
        print(name + " slow rate:" + str(slow_count/10) + "%\n")
        print(name + " very slow rate:" + str(very_slow_count/10) + "%\n")
        print(name + " ultra slow rate:" + str(ultra_slow_count/10) + "%\n")

    if "app" in name:
        result[name] = sum/count

ignore = []
ignore_file = []
multi_reader = []
related_file = []
app = [[0, 1], [3,4], [2,5]]

for i in range(6):
    exec("topic%s_recv=[-1]*300" % (i))
    exec("topic%s_sent=[-1]*300" % (i))

if len(sys.argv) > 1:
    dir = "./result/" + sys.argv[1] + "/"
else:
    dir = "./result/xxx/"

for root, dirs, files in os.walk(dir):
    for filename in files:
        if "result_unit" not in filename or filename in ignore_file:
            continue 
        if filename in related_file:
            for t in multi_reader:
                exec("%s_%s_sent=[-1]*300" % (t, filename.split('.')[0]))
                exec("%s_%s_recv=[-1]*300" % (t, filename.split('.')[0]))
        with open(dir + filename, 'r') as file:
            for line in file:
                parts = line.strip().split()
                if len(parts) == 2 and "offset" in parts[0]:
                    offset = int(parts[1])
                    print(filename + " offset:" + str(offset))
                    continue
                else:
                    topic = parts[0]
                    op = parts[1]
                    if parts[1] == "recvf":
                        op = "recv"
                    index = int(parts[2].replace("no.", "").replace(":", ""))
                    if parts[0] in multi_reader and filename in related_file:
                        parts[0] += ("_" + filename.split('.')[0])
                exec("%s_%s[index] = int(parts[-2]) + offset" % (topic, op))


for i in range(6):
    if i not in ignore:
        if "topic" + str(i) in multi_reader:
            for file in related_file:
                name = file.split('.')[0]
                exec("print('topic%s_sent missed: ' + str(topic%s_sent.count(-1)))" % (i, i))
                exec("print('topic%s_%s_recv missed: ' + str(topic%s_%s_recv.count(-1)))" % (i, name, i, name))
        else:
            exec("print('topic%s_sent missed: ' + str(topic%s_sent.count(-1)))" % (i, i))
            exec("print('topic%s_recv missed: ' + str(topic%s_recv.count(-1)))" % (i, i))
print("\n")

for i in range(6):
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

print(result)
