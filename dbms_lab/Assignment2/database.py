import MySQLdb
import pprint

conn = MySQLdb.connect(host = "10.5.18.68", user = "13CS30030", passwd = "cse12", db = "13CS30030")
cursor = conn.cursor()

pp = pprint.PrettyPrinter(indent = 4)

print "\nQuery One"

name = raw_input("Enter actor name!\n")
query = """select *
from tb_movie
where tb_movie.movID in
(
 select tb_acted.movID
 from tb_acted
 where tb_acted.actorID in
 (
 select actorID
 from tb_actor
 where tb_actor.fname= "{0}"
 and tb_actor.lname="khan"
 )
);""".format(name)
cursor.execute(query)

row = cursor.fetchall()
for result in row:
    pp.pprint(result)

print "\nQuery two"
cursor.execute("""select *
from tb_movie as M
where M.movID in
(
 select S1.movID
 from tb_acted as S1
 where S1.actorID in
 (
 select A.actorID
 from tb_actor as A
 where A.fname="sharukh"
 and A.lname="khan"
 )
 and S1.movID in
 (
 select D1.movID
 from tb_directed as D1
 where D1.dirID in
 (
 select D.dirID
 from tb_director as D
 where D.fname="sharukh"
 and D.lname="khan"
 )
 )
);""")

row = cursor.fetchall()
for result in row:
    pp.pprint(result)


print "\nQuery three"
cursor.execute("""select *
from tb_movie
where tb_movie.movID in
(
 select tb_acted.movID
 from tb_acted
 where tb_acted.actorID in
 (
 select tb_actor.actorID
 from tb_actor
 where tb_actor.fname="sharukh"
 and tb_actor.lname="khan"
 )
)
and tb_movie.genre = "thriller"
and DATEDIFF(tb_movie.year, "2000-01-01") > 0;""")

row = cursor.fetchall()
for result in row:
    pp.pprint(result)

print "\nQuery four"
cursor.execute("""select A2.actorID,A2.fname,A2.lname,A2.DOB
from tb_actor as A1, tb_acted as S1, tb_actor as A2, tb_acted as S2
where A1.actorID = S1.actorID
and A2.actorID = S2.actorID
and S1.movID = S2.movID
and A1.actorID != A2.actorID
and A1.fname = "Sharukh"
and A1.lname = "Khan"
and DATEDIFF(A2.DOB,A1.DOB) >= 3650;""")

row = cursor.fetchall()
for result in row:
    pp.pprint(result)

print "\nQuery five"
cursor.execute("""select x.dirID,x.fname,x.lname,x.DOB
from
(
 select count(D.dirID)as c,D.dirID,D.fname,D.lname,D.DOB
 from tb_actor as A,tb_acted as AC, tb_director as D, tb_directed as DR
 where A.actorID = AC.actorID
 and D.dirID = DR.dirID
 and AC.movID = DR.movID
 and A.fname = "sharukh"
 and A.lname = "khan"
 group by D.dirID
) as x
where x.c =
(
 select max(y.c1)
 from
 (
 select count(D1.dirID)as c1,D1.dirID,D1.fname,D1.lname,D1.DOB
 from tb_actor as A1,tb_acted as AC1, tb_director as D1, tb_directed as DR1
 where A1.actorID = AC1.actorID
 and D1.dirID = DR1.dirID
 and AC1.movID = DR1.movID
 and A1.fname = "sharukh"
 and A1.lname = "khan"
 group by D1.dirID
 ) as y
);""")

row = cursor.fetchall()
for result in row:
    pp.pprint(result)
