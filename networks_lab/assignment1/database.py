import MySQLdb
from prettytable import *

conn = MySQLdb.connect(host = "10.5.18.68", user = "13CS30030", passwd = "cse12", db = "13CS30030")
cursor = conn.cursor()

x = PrettyTable()
x.field_names = ["newsID", "head", "body", "news_date", "type"]

query = "SELECT * FROM tb_news \G"
cursor.execute(query)

row = cursor.fetchall()
for result in row:
    x.add_row(result)

print x.get_string(border=False, padding_width=5)
