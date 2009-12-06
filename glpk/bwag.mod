set VALUES;


param InitialLine {i in VALUES};
param InitialColumn {i in VALUES};
param Size;

set NUMBERS := 1..Size;

var column {i in NUMBERS, j in NUMBERS} binary;
var line {i in NUMBERS, j in NUMBERS} binary;
var banda >= 0;

minimize 
  maiorbanda: banda;

subject to
  UmValorColunaA {i in NUMBERS} : (sum{j in NUMBERS} column[i,j]) = 1;
  UmValorColunaB {i in NUMBERS} : (sum{j in NUMBERS} column[j,i]) = 1;
  UmValorLinhaA {i in NUMBERS}  : (sum{j in NUMBERS} line[i,j]) = 1;
  UmValorLinhaB {i in NUMBERS}  : (sum{j in NUMBERS} line[j,i]) = 1;
  LimiteBandaA {i in VALUES} : (sum{j in NUMBERS} j*column[InitialColumn[i],j]) - (sum{j in NUMBERS} j*line[InitialLine[i],j]) <= banda;
  LimiteBandaB {i in VALUES} : (sum{j in NUMBERS} j*line[InitialLine[i],j]) - (sum{j in NUMBERS} j*column[InitialColumn[i],j]) <= banda;
end;
