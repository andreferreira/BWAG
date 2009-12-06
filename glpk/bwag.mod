set VALUES;

param InitialLine {i in VALUES};
param InitialColumn {i in VALUES};

var column {i in VALUES} >=0 integer;
var line {i in VALUES} >=0 integer;
var banda >= 0;

minimize 
  maiorbanda: banda;

subject to
  LimiteBandaA {i in VALUES} : column[i] - line[i] <= banda;
  LimiteBandaB {i in VALUES} : line[i] - column[i] <= banda;
  RestricaoColuna {x in VALUES , y in VALUES} : (if InitialColumn[x] = InitialColumn[y] then column[x] else column[y]) = column[y];
  RestricaoLinha {x in VALUES , y in VALUES} : (if InitialLine[x] = InitialLine[y] then line[x] else line[y]) = line[y];
  Unicidade {x in VALUES , y in VALUES} : column[x] + card(VALUES)*line[x] != column[y] + card(VALUES)*line[y];
end;
