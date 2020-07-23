import csv

c = 0
field = {}

with open('OrgGenotypePercentiles.csv', 'r') as csvFile:
    reader = csv.reader(csvFile)
    for row in reader:
        field[c] = row
        print(field[c])
        c = c + 1

row = len(field[0])
column = len(field)

csvFile.close()
