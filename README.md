# Kliens-szerver sakk alkalmazás 
[_in English_](./README_en.md)

# Feladatkiírás
A feladat egy sakk szerver alkalmazás megvalósítása, amihez kliensek csatlakozhatnak, és játékot kezdeményezhetnek.
A szerver funkciói:
- regisztrált felhasználók nyilvántartása
- több játék párhuzamos kezelése
- a játékosok lépéseinek validálása, játékállás kezelése
- véletlenszerű ellenfél keresés

kliens:
- csatlakozás a szerverhez
- játék megjelenítése grafikus felületen, kommunikáció a szerverrel


## Függőségek
make, g++, qmake (qt5-qmake, qmake-qt5), Qt5 library

## Futtatás
A gyökérkönyvtárban a `make` parancsot kiadva fordul a kliens, és a szerver is. A szervert a **server/bin/server** állomány futtatja. Az aktuális könyvtárban létrehozza az "adatbázis" fájlt, ezért írási joga kell, hogy legyen.
A szerver indításakor a portszám opcionálisan megadható (a default 25000):
```
cd server/bin
./server [port]
```

A kliensek a **client/bin/client** útvonalon indíthatóak.
A kliens külön is fordítható, a client/bin mappában az alábbi parancsokkal:
```
qmake ../client.pro
make
```

## Példafuttatás

A szervert elindítjuk a localhoston az 54242-es porton:
```
$ lsof -iTCP -sTCP:LISTEN
COMMAND     PID USER   FD   TYPE DEVICE SIZE/OFF NODE NAME
server    14464 rego    3u  IPv4 351126      0t0  TCP *:54242 (LISTEN)
```

A kliensek bejelentkezés (bal) és játékra jelentkezés után összepárosítódnak és bekerülnek egy új játékba (jobb):

<p align="center">
     <img src="./assets/1_a_logged_in.png" width="45%">
     &nbsp; &nbsp; &nbsp; &nbsp;
     <img src="./assets/2_b_start.png" width="45%">
</p>

Első lépés (bal) majd a megnyitás (Ruy-Lopez Morphy defence) (jobb):

<p align="center">
   <img src="./assets/3_b_first_move.png" width="45%">
   &nbsp; &nbsp; &nbsp; &nbsp;
   <img src="./assets/4_a_opening.png" width="45%">
</p>

A lehetséges lépések sárgán látszódnak (bal):

<p align="center">
  <img src="./assets/5_a_moves.png" width="45%">
   &nbsp; &nbsp; &nbsp; &nbsp;
  <img alt="" src="./assets/6_b_midgame.png" width="45%">
</p>

A játszma vége, sötétnek lejárt az ideje, pedig nyerésre állt:

<p align="center">
  <img alt="" src="./assets/7_a_timeout.png" width="45%">
&nbsp; &nbsp; &nbsp; &nbsp;
  <img alt="" src="./assets/8_b_timeout.png" width="45%">
</p>


# Megvalósított program
A kiírás szerinti funciók működőképesek. A kliens-szerver közötti kommunikáció saját protokollt használ. A protokoll üzeneteit a Message leszármazott osztályai implementálják. Ez közös a szerver és a kliensek között, csak a feldolgozó függvényeket implementálják külön.

## A hálózati protokoll
A protokoll üzenetei: (minden üzenet "\n"-re végződik)
| Küldő | Tartalom | Leírás |
|:-----:|:---:|:---:|
|kliens| REGISTER \<name\> \<password\> | regisztrál a szerverre |
|kliens| LOGIN \<name\> \<password\> | bejelentkezik a szerverre |
|kliens| START | játékot indít a szerveren, csak bejelentkezve működik, ha nincs futó játéka a felhasználónak |
|kliens| MOVE \<x\> \<y\> | a játékos lép az x mezőn álló figurával az y-ra |
|szerver| OK | a kliens üzenete sikeres volt |
|szerver| ERROR \<error message\> | hibaüzenet (szövegesen) |
|szerver| GAME \<white\> \<black\> \<time_white\> \<time_black\> \<result\> | Játékstátusz frissítő üzenet. \<white\>, és \<black\> a felhasználók nevei, \<time_white\>, \<time_black\> a hátfralévő idejük ms-ban, és \<result\> jelzi, ha vége a játéknak |
|szerver| MOVED \<x\> \<y\> | A szerver elküldi az ellenfél lépését a játékosnak |

A kliens minden üzenetére a szerver OK, vagy ERROR üzenetet küld válasznak. A játék kezdetétől a végéig a szerver periodikusan küldi a GAME üzeneteket, illetve ha az ellenfél lépett, a MOVED üzenet jelzi, a játékállás nem kerül leküldésre. Az állapotot a kliensek is tárolják, de a szerver ellenőrzi a lépéseket.

A protokoll elég pazarló, a hibaüzeneteket szövegesen küldi át. A könnyű parancssoros debugolásért választottam ezt a protokollt, de egyszerű lenne áttérni egy hatékonyabbra.
