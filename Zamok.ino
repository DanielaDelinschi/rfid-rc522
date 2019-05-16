
#include <EEPROM.h> 
#include <SPI.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

#include <MFRC522.h>

#define SS_PIN 10
#define RST_PIN 9
MFRC522 mfrc522(SS_PIN, RST_PIN);  
unsigned long uidDec, uidDecTemp;  //Pentru a afișa numărul cardului în format zecimal.


//Array pentru a stoca 50 "Card UID" / numere unice ale cardurilor NFC citite de pe EEPROM.
unsigned long CardUIDeEPROMread[] = {
  0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29,
  30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49
};

int ARRAYindexUIDcard; // Index pentru matricea CardUIDeEPROMread.

int EEPROMstartAddr; // Pornirea celulei de memorie pentru scrierea / citirea EEPROM "Card UID".

long adminID = 4201436206;

int LockSwitch;// Blocare / Comutare 

int LockPin = 8;

const int buttonPin = 2;
int buttonState = 0;

void setup() {
  Serial.begin(9600);
  lcd.begin();


  SPI.begin(); //  inițializarea busului SPI 
  mfrc522.PCD_Init(); // inițializarea cardului MFRC522 
  digitalWrite(LockPin, LOW);

  pinMode(buttonPin, INPUT);
  displayWAiT_CARD(); // Rularea functiei,pornirea ecranului 

  EEPROMreadUIDcard(); //Rularea funcției pentru a suprascrie matricea CardUIDeEPROMread cu date din EEPROM.
  //Pe monitorul serial apar  toate numerele cardului UID înregistrate
  for (int i = 0; i <= 49; i++)Serial.print(i), Serial.print(" ---- "), Serial.println(CardUIDeEPROMread[i]); 

  //for (int i = 0; i < 512; i++)EEPROM.write(i, 0); // EEPROM Clear , daca doriti sa stergeti EEPROM-ul
}

void loop() {

  // Cautarea unui nou card NFC
  if ( ! mfrc522.PICC_IsNewCardPresent()) {
    return;
  }

  // Selectarea cartelei
  if ( ! mfrc522.PICC_ReadCardSerial()) {
    return;
  }

  // Eliberarea numărului de serie al cardului "UID".
  for (byte i = 0; i < mfrc522.uid.size; i++)
  {
    uidDecTemp = mfrc522.uid.uidByte[i];
    uidDec = uidDec * 256 + uidDecTemp;

  }
  
  // uidDec - Acesta este cardul UID primit / un card NFC unic.
  // Scrieți "manual" în numărul de schiță "Card UID" al cardului de administrator pentru a porni "modul de înregistrare" din utilizatorii EEPROM "Card UID".
  // Verificați dacă "Card UID" se potrivește cu "Card UID" pre-înregistrat al administratorului sau dacă LockSwitch este mai mare de 0.

  if (uidDec == 4201436206 || LockSwitch > 0)EEPROMwriteUIDcard(); //Pornim funcția de înregistrare în EEPROM "Card UID" al utilizatorilor.
       // Pentru modul de control.
  if (LockSwitch == 0) // numai dacă suntem în "modul de control".
  {
    // Iteram prin matricea CardUIDeEPROMread.
    for (ARRAYindexUIDcard = 0; ARRAYindexUIDcard <= 49; ARRAYindexUIDcard++)
    {
      // filtru, în cazul în care nu înregistrăm toate cele 50 de variabile ale matricei, în variabilele nescrise ale matricei va fi valoarea 0.
      if (CardUIDeEPROMread[ARRAYindexUIDcard] > 0) // Încadrarea prin întreaga matrice de carduri CardUIDeEPROMread, filtrarea de la 0.
      {
        //  Repetăm ​​întreaga matrice a cardului CardUIDeEPROMread, dacă cartea ridicată se potrivește cu una dintre cele 50 de variabile ale matricei.
        if (CardUIDeEPROMread[ARRAYindexUIDcard] == uidDec) // Dacă se găsește o coincidenta cu un card apropiat  
        {
          digitalWrite(LockPin, HIGH);
          CommandsCARD(); // Ruleaza funcția pentru a efectua orice acțiune, în funcție de "Card UID" al cardului prezentat.
          break; 
        }
      }
    }
    //ARRAYindexUIDcard == 50 Înseamnă că am trecut prin întreaga matrice de la 0 la 49 și nu am găsit un card cu "Card UID" prezentat.
    // Adică, cartela ridicată nu este în baza de date, afișăm cardul "nu a fost găsit" și numărul cardului "Card UID".
    if (ARRAYindexUIDcard == 50) Serial.print("NOT  Found CARD-UID  ID:"), Serial.println(uidDec), lcd.setCursor(0, 0), lcd.print("NOT Found CARD"), lcd.setCursor(0, 1), lcd.print("                "), lcd.setCursor(1, 1), lcd.print("ID:"), lcd.print(uidDec) ;
    Serial.print("LockPin=");
    Serial.println(digitalRead (LockPin));
    delay(2000);
    digitalWrite(LockPin, LOW);
    ARRAYindexUIDcard = 0;
    displayWAiT_CARD(); // Rularea functiei,pornirea ecranului 
  }
}


void EEPROMdeleteUIDcard() {
  buttonState = digitalRead(buttonPin);

   if (buttonState == HIGH) {
    EEPROM.write(EEPROMstartAddr - 5, 00);
    EEPROM.write(EEPROMstartAddr - 4, 00);
    EEPROM.write(EEPROMstartAddr - 3, 00);
    EEPROM.write(EEPROMstartAddr - 2, 00);
    EEPROM.write(EEPROMstartAddr - 1, 00);
    EEPROM.write(EEPROMstartAddr, 00);
    Serial.println("   Card Deleted   ");
    lcd.clear();
    lcd.print("Card Deleted");
  }
  else { }
}

// Efectuând o funcție, de scriere pe EEPROM "Card UID" a utilizatorilor, maximum 50 "Card UID".
void EEPROMwriteUIDcard() {

  if (LockSwitch == 0) // Dacă suntem în modul de control.
  {
    Serial.println("   START    RECORD   Card UID   CLIENT");
    lcd.setCursor(0, 0);
    lcd.print("RECORD Card UID");

  }
  // Pentru a sări peste intrarea în celula de memorie.
  if (LockSwitch > 0) // Dacă suntem în "modul de înregistrare".
  { 
   if (uidDec == 4201436206) // Dacă este prezentată cartea de administrare.
    {
      Serial.println("   SKIP     RECORD   ");
      lcd.setCursor(0, 0);
      lcd.print("  SKIP RECORD   ");
      lcd.setCursor(0, 1);
      lcd.print("                ");
      lcd.setCursor(0, 1);
      lcd.print("  label: ");
      lcd.print(EEPROMstartAddr / 5);
      EEPROMdeleteUIDcard();
      Serial.println(EEPROMstartAddr / 5); //Afișăm numărul celulei de memorie pe care o ratam
      EEPROMstartAddr += 5; //Trecem peste intrarea în celula de memorie, dacă nu dorim să scrie "Card UID" acolo.
      
    }
      else // În caz contrar, adică, un card este adus pentru înregistrare.
      // "Card UID" / numărul cardului este un "număr lung" care nu se potrivește într-o singură memorie EEPROM.
      // Am divizat "numărul lung" în 4 părți, iar în bucăți, îl scriem în 4 celule ale EEPROM. Începem înregistrarea de la adresa EEPROMstartAddr.

    { EEPROMdeleteUIDcard();
      EEPROM.write(EEPROMstartAddr, uidDec & 0xFF);
      EEPROM.write(EEPROMstartAddr + 1, (uidDec & 0xFF00) >> 8);
      EEPROM.write(EEPROMstartAddr + 2, (uidDec & 0xFF0000) >> 16);
      EEPROM.write(EEPROMstartAddr + 3, (uidDec & 0xFF000000) >> 24);
      // Înregistrat!
      delay(10);
      // --
      Serial.println("RECORD OK! IN MEMORY ");
      lcd.setCursor(0, 0);
      lcd.print("RECORD OK! ");

      Serial.println(EEPROMstartAddr / 5); //Scrieti pe monitorul serial numărul celulei de memorie înregistrate.
      lcd.setCursor(11, 0);
      lcd.print("K: ");
      lcd.print(EEPROMstartAddr / 5);
      lcd.setCursor(0, 1);
      lcd.print("ID:");
      lcd.print(uidDec);
      EEPROMdeleteUIDcard();

      EEPROMstartAddr += 5; // Adăugăm 5 la celula de pornire a înregistrării.

    }
  }

  LockSwitch++; //Deblocați modul de înregistrare și blocați modul de control.

  if (EEPROMstartAddr / 5 == 50) // daca am ajuns la 50
  {
    delay(2000);
    Serial.println("RECORD FINISH");
    lcd.setCursor(0, 0);
    lcd.print("  RECORD FINISH ");

    EEPROMstartAddr = 0;
    uidDec = 0;
    ARRAYindexUIDcard = 0;
    Serial.println("__1__");
    EEPROMreadUIDcard(); // Ruleaza funcția pentru a suprascrie matricea CardUIDeEPROMread cu date din EEPROM.
  }
}

// Realizarea funcției, rescriind matricea CardUIDeEPROMread, cu date de pe EEPROM.
void EEPROMreadUIDcard()
{
  for (int i = 0; i <= 49; i++)
  {
    byte val = EEPROM.read(EEPROMstartAddr + 3);
    CardUIDeEPROMread[ARRAYindexUIDcard] = (CardUIDeEPROMread[ARRAYindexUIDcard] << 8) | val;
    val = EEPROM.read(EEPROMstartAddr + 2);
    CardUIDeEPROMread[ARRAYindexUIDcard] = (CardUIDeEPROMread[ARRAYindexUIDcard] << 8) | val;
    val = EEPROM.read(EEPROMstartAddr + 1); // увеличиваем EEPROMstartAddr на 1.
    CardUIDeEPROMread[ARRAYindexUIDcard] = (CardUIDeEPROMread[ARRAYindexUIDcard] << 8) | val;
    val = EEPROM.read(EEPROMstartAddr);
    CardUIDeEPROMread[ARRAYindexUIDcard] = (CardUIDeEPROMread[ARRAYindexUIDcard] << 8) | val;

    ARRAYindexUIDcard++; // marim cu 1
    EEPROMstartAddr += 5; //marim cu 5
  }

  ARRAYindexUIDcard = 0;
  EEPROMstartAddr = 0;
  uidDec = 0;
  LockSwitch = 0;
  displayWAiT_CARD();
}

// Realizarea funcției, afisarea pe ecran.
void displayWAiT_CARD()
{
  lcd.setCursor(0, 0), lcd.print("   ATTACH THE   ");
  lcd.setCursor(0, 1), lcd.print("      CARD      ");

}

// -------------------------------------------

//Avem o funcție de a efectua orice acțiune, în funcție de "Card UID" al cardului deținut.
void CommandsCARD()
{
  Serial.println("--------------------");
  Serial.print("Hi "); // Afișează "Hi ".
  lcd.setCursor(0, 0), lcd.print(" Hi,  "), lcd.print(ARRAYindexUIDcard), lcd.print("        ");
  lcd.setCursor(1, 1), lcd.print("ID:"), lcd.print(uidDec) ;
   // În "ARRAYindexUIDcard" va fi stocat temporar numărul de index al matricei, care coincide cu valoarea cardului UID ridicat.
  // De exemplu, dacă cardul-UID ridicat a coincis cu cardul UID înregistrat în celula nr. 0 / indexul arrayului ARRAYindexUIDcard nr. 0.
  if (ARRAYindexUIDcard == 0)
  {
    Serial.println("Daniela"); // Afișează "Daniela".
    lcd.setCursor(6, 0), lcd.print("Daniela  ");
    // ! Aici puteți adăuga orice alte acțiuni.
  }

  else if (ARRAYindexUIDcard == 1) // Dacă cardul UID este obținut din celula nr. 1 / indexul matricei nr. 1.
  {
    Serial.println("Adelina");
    lcd.setCursor(6, 0), lcd.print("Adelina ");
  }

  else if (ARRAYindexUIDcard == 2) // Dacă cardul UID este obținut din celula nr. 2 / indexul matricei nr. 2.
  {
     Serial.println("Andrei");
    lcd.setCursor(6, 0), lcd.print("Andrei");
  }

  else if (ARRAYindexUIDcard == 3)
  {
     Serial.println("Maria");
    lcd.setCursor(6, 0), lcd.print("Maria");
  }
  //.......................
  else if (ARRAYindexUIDcard == 49)
  {
    Serial.println("Alexandru");
  }


  Serial.print("ID: ");
  Serial.println(CardUIDeEPROMread[ARRAYindexUIDcard]); // Afișeaza cardul UID detectat.

  Serial.print("USER № ");
  Serial.println(ARRAYindexUIDcard); // Afișeaza numărul index al matricei / celui în care a fost stocat cardul UID detectat.


}
