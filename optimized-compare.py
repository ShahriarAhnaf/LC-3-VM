Lefile = open("log_speed.txt", 'r')
firstcount = []
for i in range(0,16):
   firstcount.append(0) 

line_count = 0
for line in Lefile:
    leLine = (line.split('.')[0]).split()
    speed = leLine[-1]
    #find which list to add to. 
    op=int(leLine[1][0],16)
    #use the list indext to go to op. 
    firstcount[op] += int(speed, 10)/100 # last thing in line
    line_count += 1
    if line_count > 10000: break
Lefile.close()


# other file
Lefile = open("slow_log_speed.txt", 'r')

count = []
for i in range(0,16):
   count.append(0) 
line_count = 0
for line in Lefile:
    leLine = (line.split('.')[0]).split()
    speed = leLine[-1]
    #find which list to add to. 
    op=int(leLine[1][0],16)
    #use the list indext to go to op. 
    count[op] += int(speed, 10)/100 # last thing in line
    line_count += 1
    if line_count > 10000: break
Lefile.close()


#nice output 
print(f"{'slow :':<25} {'Fast:':<30} {'DIFF':<60}")
    
for i in range(0,len(firstcount)):
    diff = firstcount[i]/count[i] * 100
    #print(f"{'Location: ' + location:<25} Revision: {revision}")
    print(f"{str(count[i]):<25} { str(firstcount[i]):<30} {str(diff) + '%':<60}")
