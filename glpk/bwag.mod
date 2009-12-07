set VALUES;


param InitialLine {i in VALUES};
param InitialColumn {i in VALUES};
param Size;

set NUMBERS := 1..Size;

var neworder {i in NUMBERS, j in NUMBERS} binary;
var banda >= 0;

minimize 
  maiorbanda: banda;

subject to
  UmValorA {i in NUMBERS} : (sum{j in NUMBERS} neworder[i,j]) = 1;
  UmValorB {i in NUMBERS} : (sum{j in NUMBERS} neworder[j,i]) = 1;
  LimiteBandaA {i in VALUES} : (sum{j in NUMBERS} j*neworder[InitialColumn[i],j]) - (sum{j in NUMBERS} j*neworder[InitialLine[i],j]) <= banda;
  LimiteBandaB {i in VALUES} : (sum{j in NUMBERS} j*neworder[InitialLine[i],j]) - (sum{j in NUMBERS} j*neworder[InitialColumn[i],j]) <= banda;
end;
