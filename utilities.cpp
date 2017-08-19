#include "utilities.h"

QString nameDay(int num) {
    switch(num) {
        case 0: return "Lundi";
        case 1: return "Mardi";
        case 2: return "Mercredi";
        case 3: return "Jeudi";
        case 4: return "Vendredi";
        case 5: return "Samedi";
        case 6: return "Dimanche";
    }
    return "";
}
