//Calculator.ino
#include "Display.h"
#include "Keypad.h"




// calculator state
String historyLine="", calcLine="", alternateDisplay="";
bool justCalculated=false, usedFractionEntry=false;
Fraction currentFraction(0,1), storedFraction(0,1);
enum Operation { OP_NONE, OP_ADD, OP_SUBTRACT, OP_MULTIPLY, OP_DIVIDE };
Operation currentOperation = OP_NONE;





void clearAll() {
  calcLine = "";
  historyLine = "";
  alternateDisplay = "";
  justCalculated = false;
  currentFraction = Fraction(0, 1);
  storedFraction = Fraction(0, 1);
  currentOperation = OP_NONE;
  usedFractionEntry = false;
  // Clear display area
  clearDisplayArea();
}








void handleCalculatorMode(const char* key) {
  // Quick aliases
  char first = key[0];

  // AC
  if (strcmp(key, "AC") == 0) {
    clearAll();
    return;
  }
  // Mixed-number space
  if (strcmp(key, ">") == 0) {
    if (calcLine.length() && calcLine.endsWith(" ") == false)
      calcLine += " ";
    return;
  }
  // x/y fraction
  if (strcmp(key, "x/y") == 0) {
    usedFractionEntry = true;
    if (calcLine.length() && calcLine.endsWith("/") == false && calcLine.endsWith(" ") == false)
      calcLine += "/";
    return;
  }
  // Delete
  if (strcmp(key, "Del") == 0) {
    if (!justCalculated && calcLine.length())
      calcLine.remove(calcLine.length() - 1);
    return;
  }

  // MM ↔ in modifier
  if (strcmp(key, "MM") == 0) {
    if (calcLine.length()) {
      double val = evaluateExpression(calcLine);
      if (zeroHoldActive) {
        double inches = val / 25.4;
        calcLine = formatAnswer(inches) + " in";
      } else {
        double mm = val * 25.4;
        calcLine = formatAnswer(mm) + " mm";
      }
      justCalculated = true;
      zeroHoldActive = false;  // reset
    }
    return;
  }

  // round (toggle fraction/decimal)
  if (strcmp(key, "round") == 0) {
    if (calcLine.length()) {
      if (containsFraction(calcLine)) {
        double v = evaluateExpression(calcLine);
        calcLine = formatAnswer(v);
      } else {
        double v = evaluateExpression(calcLine);
        Fraction f = decimalToSixtyFourths(v);
        calcLine = formatMixedFraction(f);
      }
      justCalculated = true;
    }
    return;
  }

  // After a result, new operator or digit starts fresh
  if (justCalculated) {
    if (isOperator(key)) {
      calcLine += key;
      justCalculated = false;
      return;
    }
    if (isNumber(key)) {
      calcLine = String(key);
      justCalculated = false;
      usedFractionEntry = false;
      return;
    }
  }

  // Digit
  if (isNumber(key)) {
    calcLine += key;
    return;
  }
  // Operator
  if (isOperator(key)) {
    calcLine += key;
    return;
  }
  // Decimal point
  if (first == '.') {
    calcLine += key;
    return;
  }
  // Calculate
  if (strcmp(key, "return") == 0) {
    if (calcLine.length() && !justCalculated) {
      String expr = calcLine;
      double ans = evaluateExpression(expr);
      historyLine = expr;

      if (usedFractionEntry) {
        Fraction f = decimalToSixtyFourths(ans);
        if (ans == f.toDecimal()) {
          calcLine = formatMixedFraction(f);
          alternateDisplay = formatAnswer(ans);
        } else {
          calcLine = formatAnswer(ans);
          alternateDisplay = "";
        }
      } else {
        calcLine = formatAnswer(ans);
        Fraction f = decimalToSixtyFourths(ans);
        if (ans == f.toDecimal()) {
          alternateDisplay = formatMixedFraction(f);
        } else {
          alternateDisplay = "";
        }
      }

      justCalculated = true;
      usedFractionEntry = false;
    }
    return;
  }
}








// Expression evaluator & helpers
double evaluateExpression(const String& expr) {
  
  String s = expr;
  s.replace("x", "*");
  double result = 0;
  char op = '+';
  int start = 0;
  for (int i = 0; i <= s.length(); i++) {
    char c = (i < s.length()) ? s[i] : '\0';
    bool isOp = (c == '+' || c == '-' || c == '*' || c == '\0');
    if (isOp) {
      if (i > start) {
        String tok = s.substring(start, i);
        double val = parseMixedFractionValue(tok);
        switch (op) {
          case '+': result += val; break;
          case '-': result -= val; break;
          case '*': result *= val; break;
          case '/':
            if (val != 0) result /= val;
            break;
        }
      }
      op = c;
      start = i + 1;
    }
  }
  return result;
}




double parseMixedFractionValue(const String& token) {
  int sp = token.indexOf(' ');
  if (sp >= 0) {
    double whole = token.substring(0, sp).toFloat();
    double frac = parseFraction(token.substring(sp + 1));
    return (whole < 0) ? whole - frac : whole + frac;
  }
  if (token.indexOf('/') >= 0) return parseFraction(token);
  return token.toFloat();
}




double parseFraction(const String& frac) {
  int slash = frac.indexOf('/');
  double n = frac.substring(0, slash).toFloat();
  double d = frac.substring(slash + 1).toFloat();
  return (d == 0) ? 0 : n / d;
}

bool containsFraction(const String& expr) {
  if (expr.indexOf(' ') >= 0 && expr.indexOf('/') >= 0) return true;
  return expr.indexOf('/') >= 0;
}

String formatAnswer(double v) {
  if (abs(v - round(v)) < 0.001) return String((long)round(v));
  String out = String(v, 8);
  while (out.endsWith("0")) out.remove(out.length() - 1);
  if (out.endsWith(".")) out.remove(out.length() - 1);
  return out;
}



inline bool isNumber(const char* k) {
  return (k[0] >= '0' && k[0] <= '9');
}
inline bool isOperator(const char* k) {
  return (k[0] == '+' || k[0] == '-' || k[0] == '*' || k[0] == '/');
}

