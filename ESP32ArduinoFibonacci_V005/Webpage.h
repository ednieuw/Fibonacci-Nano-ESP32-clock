const char* PARAM_INPUT_1 = "input1";
const char index_html[]  = 
R"rawliteral(
<!DOCTYPE HTML><html><head>
  <title>Nano ESP32 Fibonacci clock</title>
  <meta name="google" content="notranslate" />
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style type="text/css">
  .auto-style1 { font-family: Verdana, Geneva, Tahoma, sans-serif; background-color: #FFFFFF;  }
  .auto-style2 { text-align: center; }
  .auto-style3 { font-family: Verdana, Geneva, Tahoma, sans-serif; background-color: #FFFFFF; color: #FF0000;  }
  .auto-style11 {	border-left: 0px solid #000000;	border-right: 0px solid #000000;	border-top: thin solid #000000;
	                border-bottom: thin solid #000000;	font-family: Verdana, Arial, Helvetica, sans-serif;  }
  .style1 {font-size: smaller}
  .style2 {font-size: small}
  .style3 {	font-family: "Courier New", Courier, mono;	font-weight: bold;}
  .style4 {	color: #999999;	font-weight: bold;}
  .style5 {	color: #FF0000;	font-weight: bold;	font-size: smaller;}
  .style7 {	color: #00FF00;	font-weight: bold;	font-size: smaller;}
  .style8 {	color: #0000FF;	font-weight: bold;	font-size: smaller;}
  .style9 {color: #0066CC}
  .style10 {font-family: "Courier New", Courier, mono; font-weight: bold; color: #0066CC; }  </style>
  </head>
  <body>
   <table style="width: auto" class="auto-style11">
     <tr>
       <td colspan="3" class="auto-style2">
   <span class="auto-style3">
     <a href="https://github.com/ednieuw/Arduino-ESP32-Nano-Fibonacci clock">ESP32-Nano Word Clock</a> 
     <tr>
       <td width="123" style="width: 115px"> <strong>A</strong> SSID</td>
       <td width="98" style="width: 115px"><strong>B</strong> Password</td>
       <td width="157" style="width: 115px"><strong>C</strong> BLE beacon</td>
     </tr>
     <tr>
       <td colspan="3"><strong>D</strong> Date <span class="auto-style4 style1">(D15012021)</span>
                       <strong>&nbsp; T</strong> Time                <span class="auto-style4 style1">(T132145)</span></td>
     </tr>
     <tr>
       <td colspan="3"><strong>E</strong> Set Time zone <span class="auto-style4 style1"> E<-02>2 or E<+01>-1</span></td>
     </tr>
     <tr>
       <td colspan="3"><strong>F</strong> Fibonacci or Chrono display</td>
     </tr>
     <tr>
       <td colspan="3"><strong>H</strong> Toggle use rotary encoder </td>
     </tr>
     <tr>
       <td colspan="3"><strong>J</strong> Toggle use of DS3231 time module </td>
     </tr>
     <tr>
       <td colspan="3"><strong>N</strong> Display Off between Nhhhh (N2208)</td>
     </tr>
     <tr>
       <td colspan="3"><strong>O</strong> Display toggle On/Off</td>
     </tr>
     <tr>
       <td colspan="3"> <strong>P</strong> Status LED toggle On/Off</td>
     </tr>
     <tr>
       <td colspan="3"> <strong>U</strong> Demo mode U0-U999 (U100)</td>
     </tr>  
     <tr>
       <td colspan="3"> <strong>V</strong> Normal, Extreme or Ultimate mode</td>
     </tr>     
     <tr>
       <td colspan="3"> <div align="center" class="style10">Display colour palettes</div></td>
     </tr>
          <tr>
       <td colspan="3"> <strong>Q0, Q1, Q8, Q9</strong>  Mondriaan styles</td>
     </tr>
     <tr>
       <td style="width: 115px"><strong>Q2</strong> RGB</td>
       <td style="width: 115px"><strong>Q3</strong> Greens</td>
       <td style="width: 115px"><strong>Q4</strong> Pastel</td>

     </tr>
     <tr>
       <td style="width: 115px"><strong>Q5</strong> Modern</td>
       <td style="width: 115px"><strong>Q6</strong> Cold</td>
       <td style="width: 115px"><strong>Q7</strong> Warm</td>
     </tr>     
	  <tr>
       <td colspan="3"> <div align="center" class="style9"> <span class="style3">Light intensity setting</span> <span class="style1">(0-255) </span></div></td>
     </tr>
     <tr>
       <td style="width: 115px"><strong>S</strong> Slope </td>
       <td style="width: 115px"><strong>L</strong> Min </td>
       <td style="width: 115px"><strong>M</strong> Max </td>
     </tr>
     <tr>
       <td colspan="3"> <div align="center" class="style10">Communication</div></td>
     </tr>
     <tr>
       <td style="width: 115px"><strong>W</strong> WIFI</td>
       <td style="width: 115px"> <strong>X</strong> NTP </td>
       <td style="width: 115px"><strong>CCC</strong> BLE</td>
     </tr>
     <tr>
       <td style="width: 115px"><strong>R</strong> Reset</td>
       <td style="width: 115px"><strong>@</strong> Restart </td>
       <td style="width: 115px"><strong>+</strong> Fast BLE</td>
     </tr>
      <tr>
       <td style="width: 115px"><strong>#</strong> Self test</td>
       <td style="width: 115px"><strong>!</strong> See RTC</td>
       <td style="width: 115px"><strong>&</strong> Update RTC</td>
     </tr>
     <tr>
      <td style="width: 115px"><a href="https://github.com/ednieuw/Fibonacci-Nano-ESP32-clock">Manual</a></td>
      <td style="width: 115px">&nbsp;</td>
      <td style="width: 115px"><a href="https://www.ednieuw.nl" class="style2">ednieuw.nl</a></td>   
     </tr>
   </table>
   <form action="/get">
       <strong>     
       <input name="submit" type="submit" class="auto-style3" style="height: 22px" value="Send">
       </strong>&nbsp;
     <input type="text" name="input1" style="width: 272px"></form><br>
    
<br>
</body></html>
)rawliteral";
