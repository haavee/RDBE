* FREJA.CTL - Control file for FREJA
*
* 1. Command, the pattern freja will grep the log file for.
* 2. Parameter, the column of comma-separated data after the command.
* 3. Description, the menu label freja will use for the command.
* 4. String, a level-2 grep. This parameter is optional and may be left out.
*
* NB! This file is space-separated. That is, no field may contain spaces.
*
*
* 1:Command     2:Parameter   3:Description     4:String
* -------------------------------------------------------------------------
*
  wx/           1             Temperature
  wx/           2             Pressure
  wx/           3             Humidity
  cable/        1             Cable-length
  tsys/         2             tsys1             2u
  tsys/         2             tsys9             9u
  pcal/       3             pcalamp           
  pcal/       4             pcalphs           
  tpgain/ic    2            ic_level
  tpgain/ia    2            ia_level
  "             0             Comments
*
*
