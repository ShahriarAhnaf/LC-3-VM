Lefile = open("log_speed.txt", 'r')
fastcount = []
fastcountavg = []
for i in range(0,16):
   fastcount.append(0)
   fastcountavg.append(0) 

line_count = 0
for line in Lefile:
    leLine = (line.split('.')[0]).split()
    speed = leLine[-1]
    #find which list to add to. 
    op=int(leLine[1][0],16)
    fastcountavg[op]+= 1
    #use the list indext to go to op. 
    fastcount[op] += int(speed, 10)/10 # last thing in line
    line_count += 1
    if line_count > 1000000: break
Lefile.close()


# other file
Lefile = open("slow_log_speed.txt", 'r')

slowcount = []
slowcountavg = []
for i in range(0,16):
    slowcount.append(0)
    slowcountavg.append(0)
line_count = 0
for line in Lefile:
    leLine = (line.split('.')[0]).split()
    speed = leLine[-1]
    #find which list to add to. 
    op=int(leLine[1][0],16)
    slowcountavg[op]+= 1
    #use the list indext to go to op. 
    slowcount[op] += int(speed, 10)/10 # last thing in line
    line_count += 1
    if line_count > 1000000: break
Lefile.close()


#nice output 
print(f"{'ORG(uS) :':<25} {'Optimized(uS):':<30} {'DIFF(%)':<60}")
    
for i in range(0,len(fastcount)):
    #weighted the counting by their own occurences
    if fastcountavg[i] != 0 :
        fastcount[i] /= fastcountavg[i]
    else:
        fastcountavg[i] = 0

    if slowcountavg[i] != 0:
        slowcount[i] /= slowcountavg[i]
    else:
        slowcountavg[i] =0

    if(slowcount[i] != 0):
        diff = (fastcount[i]/slowcount[i] * 100)-100
    else:
        diff = 0
    #print(f"{'Location: ' + location:<25} Revision: {revision}")
    print(f"{str(slowcount[i]):<25} { str(fastcount[i]):<30} {str(diff) + '%':<60}")



