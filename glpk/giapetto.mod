       #
       # Giapetto's problem
       #
       # This finds the optimal solution for maximizing Giapetto's profit
       #
 
       /* Set of toys */
       set TOY;
 
      /* Parameters */
      param Finishing_hours {i in TOY};
      param Carpentry_hours {i in TOY};
      param Demand_toys     {i in TOY};
      param Profit_toys     {i in TOY};

      /* Decision variables */
      var x {i in TOY} >=0;

      /* Objective function */
      maximize z: sum{i in TOY} Profit_toys[i]*x[i];

      /* Constraints */
      s.t. Fin_hours : sum{i in TOY} Finishing_hours[i]*x[i] <= 100;
      s.t. Carp_hours : sum{i in TOY} Carpentry_hours[i]*x[i] <= 80;
      s.t. Dem {i in TOY} : x[i] <= Demand_toys[i];


      data;
      /* data  section */

      set TOY := soldier train;

      param Finishing_hours:=
      soldier         2
      train           1;

      param Carpentry_hours:=
      soldier         1
      train           1;

      param Demand_toys:=
      soldier        40
      train    6.02E+23;

      param Profit_toys:=
      soldier         3
      train           2;

      end;

