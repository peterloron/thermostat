  
  /*
   * Inputs ADC Value from Thermistor and outputs Temperature in Celsius
   *  requires: include <math.h>
   * Utilizes the Steinhart-Hart Thermistor Equation:
   *    Temperature in Kelvin = 1 / {A + B[ln(R)] + C[ln(R)]3}
   *    where A = 0.001129148, B = 0.000234125 and C = 8.76741E-08
   *
   * These coefficients seem to work fairly universally, which is a bit of a 
   * surprise. 
   *
   * Schematic:
   *   [Ground] -- [10k-pad-resistor] -- | -- [thermistor] --[Vcc (5 or 3.3v)]
   *                                     |
   *                                Analog Pin 0
   *
   * In case it isn't obvious (as it wasn't to me until I thought about it), the analog ports
   * measure the voltage between 0v -> Vcc which for an Arduino is a nominal 5v, but for (say) 
   * a JeeNode, is a nominal 3.3v.
   *
   * The resistance calculation uses the ratio of the two resistors, so the voltage
   * specified above is really only required for the debugging that is commented out below
   *
   * Resistance = (1024 * PadResistance/ADC) - PadResistor 
   *
   * I have used this successfully with some CH Pipe Sensors (http://www.atcsemitec.co.uk/pdfdocs/ch.pdf)
   * which be obtained from http://www.rapidonline.co.uk.
   *
   */
  
  #include <math.h>
  #include <chibi.h>
  #include <stdlib.h>

  #define ThermistorPIN 0    // Analog Pin 0

  float vcc = 5.00;                       // only used for display purposes, if used set to the measured Vcc.
  float pad = 10000;                      // balance/pad resistor value, set this to the measured resistance of your pad resistor
  float thermr = 10000;                   // thermistor nominal resistance
  const int relay_pin = 9;
  boolean relay_on = false;
  
  const float relay_on_temp = 28.0;
  const float relay_off_temp = 29.0;
  
  float Thermistor(int RawADC) {
    long Resistance;  
    float Temp;  // Dual-Purpose variable to save space.
  
    Resistance=((1024 * pad / RawADC) - pad); 
    Temp = log(Resistance); // Saving the Log(resistance) so not to calculate  it 4 times later
    Temp = 1 / (0.001129148 + (0.000234125 * Temp) + (0.0000000876741 * Temp * Temp * Temp));
    Temp = Temp - 273.15;  // Convert Kelvin to Celsius                      
    return Temp;                                      // Return the Temperature
  }
  
  void setup() {
    //configure chibi radio
    chibiInit();
    chibiSetChannel(20);
    //chibiSleepRadio(true);
    
    //configure serial
    Serial.begin(115200);
    
    //configure relay pin
    pinMode(relay_pin,OUTPUT);
    digitalWrite(relay_pin,LOW);
  }
  
  void loop() {
    float temp;
    char buffer[16];
    
    Serial.print("Float : ");
    Serial.println(buffer);
    
    temp=Thermistor(analogRead(ThermistorPIN));       // read ADC and  convert it to Celsius
    dtostrf(temp,5,2,buffer);

    
    //transmit the data
    chibiTx(BROADCAST_ADDR, (byte*)buffer, sizeof(buffer)+1);
    //chibiTx(BROADCAST_ADDR,msg,12);
    
    Serial.print("Sending ");
    Serial.println(temp,1);
   
    //manage relay
    if(temp < relay_on_temp) {
      relay_on = true;
      digitalWrite(relay_pin,HIGH);
    }
    
    if(temp > relay_off_temp) {
      relay_on = false;
      digitalWrite(relay_pin,LOW);
    }
   
    
    delay(1000);
  }
 
