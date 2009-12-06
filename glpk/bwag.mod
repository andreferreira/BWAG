set VALUES;
set NUMBERS := 1..card(VALUES);

param InitialLine {i in VALUES};
param InitialColumn {i in VALUES};

var column {i in VALUES, j in NUMBERS} binary;
var line {i in VALUES, j in NUMBERS} binary;
var banda >= 0;

minimize 
  maiorbanda: banda;

subject to
  UmValorColuna {i in VALUES} : (sum{j in NUMBERS} column[i,j]) = 1;
  UmValorLinha {i in VALUES}  : (sum{j in NUMBERS} line[i,j]) = 1;
  LimiteBandaA {i in VALUES} : (sum{j in NUMBERS} j*column[i,j]) - (sum{j in NUMBERS} j*line[i,j]) <= banda;
  LimiteBandaB {i in VALUES} : (sum{j in NUMBERS} j*line[i,j]) - (sum{j in NUMBERS} j*column[i,j]) <= banda;
  RestricaoColuna {x in VALUES , y in VALUES} : 
     (if InitialColumn[x] = InitialColumn[y] 
       then (sum{j in NUMBERS} j*column[x,j])
       else (sum{j in NUMBERS} j*column[y,j])) 
     = (sum{j in NUMBERS} j*column[y,j]);
  RestricaoLinha {x in VALUES , y in VALUES} : 
     (if InitialLine[x] = InitialLine[y] 
       then (sum{j in NUMBERS} j*line[x,j])
       else (sum{j in NUMBERS} j*line[y,j])) 
     = (sum{j in NUMBERS} j*line[y,j]);
  Unicidade {x in VALUES, y in VALUES} :
      if InitialColumn[x] = InitialColumn[y] and InitialLine[x] = InitialLine[y] 
      then _
      else 
end;
