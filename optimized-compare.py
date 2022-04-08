import os

Lefile = open("log_speed.txt", 'r')
count = 0 
line_count = 0
for line in Lefile:
    leLine = line.split('.')
    THEline = leLine[0].split()
    count += int(THEline[-1], 10)//10000 # last thing in line
    line_count += 1
    if line_count > 10000: break
Lefile.close()
print(count)
print("\n")
count = 0.00 
line_count = 0
# other file
Lefile = open("slow_log_speed.txt", 'r')


other_count = 0.00 
line_count = 0
for line in Lefile:
    leLine = line.split('.')
    THEline = leLine[0].split()
    other_count += int(THEline[-1], 10)//10000 # last thing in line
    line_count += 1
    if line_count > 10000: break

print(other_count)
print("\n")
Lefile.close()