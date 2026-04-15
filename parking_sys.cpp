#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <iomanip>
#include <ctime>
#include <algorithm>

using namespace std;

// ============================================================
//  ENUMS
// ============================================================

enum VehicleType { Car, Bike, Truck };
enum SpotSize    { Small, Medium, Large };

string getVehicleTypeName(VehicleType t) {
    if (t == Car)   return "Car";
    if (t == Bike)  return "Bike";
    if (t == Truck) return "Truck";
    return "Unknown";
}

string getSpotSizeName(SpotSize s) {
    if (s == Small)  return "Small";
    if (s == Medium) return "Medium";
    if (s == Large)  return "Large";
    return "Unknown";
}

SpotSize getRequiredSpot(VehicleType t) {
    if (t == Bike)  return Small;
    if (t == Truck) return Large;
    return Medium;
}

double getHourlyRate(VehicleType t) {
    if (t == Bike)  return 10.0;
    if (t == Truck) return 40.0;
    return 20.0;
}

// ============================================================
//  STRUCTURES
// ============================================================

struct ParkingSpot {
    int      id;
    SpotSize size;
    bool     isOccupied;
    string   occupantPlate;

    ParkingSpot() {
        id            = 0;
        isOccupied    = false;
        occupantPlate = "";
    }

    ParkingSpot(int spotId, SpotSize spotSize) {
        id            = spotId;
        size          = spotSize;
        isOccupied    = false;
        occupantPlate = "";
    }
};

struct Vehicle {
    string      plate;
    VehicleType type;
    time_t      entryTime;
    int         spotId;

    Vehicle() {
        spotId    = 0;
        entryTime = 0;
    }

    Vehicle(string licensePlate, VehicleType vehicleType, int assignedSpot) {
        plate     = licensePlate;
        type      = vehicleType;
        spotId    = assignedSpot;
        entryTime = time(0);
    }
};

struct Ticket {
    int         ticketId;
    string      plate;
    VehicleType vehicleType;
    int         spotId;
    time_t      entryTime;
    time_t      exitTime;
    double      fee;

    Ticket() {
        ticketId = 0;
        spotId   = 0;
        fee      = 0.0;
    }
};

// ============================================================
//  HELPER FUNCTIONS
// ============================================================

string formatTime(time_t t) {
    char buffer[32];
    tm* timeInfo = localtime(&t);
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeInfo);
    return string(buffer);
}

double calculateFee(VehicleType type, time_t entryTime, time_t exitTime) {
    double seconds = difftime(exitTime, entryTime);
    double hours   = seconds / 3600.0;
    if (hours < 0) hours = 0;
    if (hours < 1) hours = 1;
    return hours * getHourlyRate(type);
}

void printLine(char ch = '-', int len = 55) {
    for (int i = 0; i < len; i++) cout << ch;
    cout << endl;
}

// ============================================================
//  DASHBOARD HELPER FUNCTIONS
// ============================================================

void clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

enum ColorCode {
    RESET   = 0,
    BLACK   = 30, RED = 31, GREEN = 32, YELLOW = 33, BLUE = 34,
    MAGENTA = 35, CYAN = 36, WHITE = 37,
    B_RED   = 41, B_GREEN = 42, B_YELLOW = 43, B_BLUE = 44,
    B_CYAN  = 46
};

void setColor(int code) {
    cout << "\033[" << code << "m";
}

void resetColor() {
    setColor(RESET);
}

void printBoxedTitle(const string& title, int width = 55) {
    string border(width, '=');
    int pad = (width - title.length() - 4) / 2;
    string spaces(pad, ' ');
    
    cout << border << endl;
    cout << "|" << spaces << title << spaces << "|" << endl;
    if ((title.length() + 4) % 2 != 0) spaces += " ";
    cout << "|" << spaces << string(title.length() + 4, '=') << "|" << endl;
    cout << border << endl;
}

void drawProgressBar(double percent, int width = 20, int fillColor = GREEN, int bgColor = 0) {
    int filled = (int)(percent * width);
    int empty = width - filled;
    
    cout << "[";
    if (fillColor != 0) setColor(fillColor);
    for (int i = 0; i < filled; ++i) cout << "█";
    resetColor();
    for (int i = 0; i < empty; ++i) cout << "░";
    cout << "] " << fixed << setprecision(1) << percent * 100 << "%" << endl;
}

void drawParkingGrid(const vector<ParkingSpot>& spots, const map<string, Vehicle>& vehicles, int cols = 6) {
    // Group spots: Small (rows 0-1), Medium (2-3), Large (4)
    vector<vector<ParkingSpot*>> grid(5, vector<ParkingSpot*>(cols, nullptr));
    int idx = 0;
    
    cout << "\n 🅿️  PARKING GRID (S=Small/Bike 🅱️, M=Medium/Car 🚗, L=Large/Truck 🚚 )" << endl;
    printLine('─', 45);
    
    for (int row = 0; row < 5; ++row) {
        cout << " │";
        for (int col = 0; col < cols; ++col) {
            if (idx < spots.size()) {
                ParkingSpot* spot = &spots[idx++];
                bool occupied = spot->isOccupied;
                string plate = spot->occupantPlate;
                string sym;
                ColorCode color = occupied ? GREEN : CYAN;
                
                if (occupied && vehicles.count(plate)) {
                    VehicleType t = vehicles.at(plate).type;
                    if (t == Bike) { sym = "🅱"; color = CYAN; }
                    else if (t == Car) { sym = "🚗"; color = GREEN; }
                    else { sym = "🚚"; color = YELLOW; }
                } else {
                    sym = occupied ? "██" : "□□";
                }
                
                setColor(color);
                cout << " " << sym << " ";
                resetColor();
            } else {
                cout << "     ";
            }
            cout << "│";
        }
        cout << endl;
    }
    printLine('─', 45);
    
    // Spot legend
    cout << " Spots: ";
    setColor(CYAN); cout << "🅱"; resetColor(); cout << " Bike, ";
    setColor(GREEN); cout << "🚗"; resetColor(); cout << " Car, ";
    setColor(YELLOW); cout << "🚚"; resetColor(); cout << " Truck | ";
    setColor(RED); cout << "██"; resetColor(); cout << " Occupied, □□ Free" << endl;
}

// ============================================================
//  PARKING LOT CLASS
// ============================================================

class ParkingLot {
private:
    string               name;
    vector<ParkingSpot>  spots;
    map<string, Vehicle> parkedVehicles;
    vector<Ticket>       ticketHistory;
    int                  nextTicketId;
    int                  totalServed;
    double               totalRevenue;

    ParkingSpot* findFreeSpot(SpotSize required) {
        for (int i = 0; i < (int)spots.size(); i++) {
            if (spots[i].size == required && !spots[i].isOccupied) {
                return &spots[i];
            }
        }
        return NULL;
    }

public:
    ParkingLot(string lotName, int smallCount, int mediumCount, int largeCount) {
        name         = lotName;
        nextTicketId = 1;
        totalServed  = 0;
        totalRevenue = 0.0;

        int id = 1;
        for (int i = 0; i < smallCount;  i++, id++) spots.push_back(ParkingSpot(id, Small));
        for (int i = 0; i < mediumCount; i++, id++) spots.push_back(ParkingSpot(id, Medium));
        for (int i = 0; i < largeCount;  i++, id++) spots.push_back(ParkingSpot(id, Large));

        cout << "\n[INIT] " << name << " is ready." << endl;
        cout << "       Small spots  : " << smallCount  << endl;
        cout << "       Medium spots : " << mediumCount << endl;
        cout << "       Large spots  : " << largeCount  << endl;
    }

    bool parkVehicle(string plate, VehicleType type) {
        if (parkedVehicles.count(plate) > 0) {
            cout << "\n[ERROR] Vehicle " << plate << " is already parked!" << endl;
            return false;
        }

        SpotSize required = getRequiredSpot(type);
        ParkingSpot* spot = findFreeSpot(required);

        if (spot == NULL) {
            cout << "\n[FULL] No " << getSpotSizeName(required)
                 << " spot available for " << getVehicleTypeName(type) << "." << endl;
            return false;
        }

        spot->isOccupied    = true;
        spot->occupantPlate = plate;
        parkedVehicles[plate] = Vehicle(plate, type, spot->id);
        totalServed++;

        printLine();
        cout << "  [ENTRY] Vehicle Parked Successfully!" << endl;
        printLine();
        cout << "  Plate   : " << plate                              << endl;
        cout << "  Type    : " << getVehicleTypeName(type)           << endl;
        cout << "  Spot No : " << spot->id
             << " (" << getSpotSizeName(spot->size) << ")"           << endl;
        cout << "  Time    : " << formatTime(parkedVehicles[plate].entryTime) << endl;
        cout << "  Rate    : Rs." << getHourlyRate(type) << " /hr (min 1 hr)" << endl;
        printLine();
        return true;
    }

    bool exitVehicle(string plate) {
        if (parkedVehicles.count(plate) == 0) {
            cout << "\n[ERROR] Vehicle " << plate << " not found in parking." << endl;
            return false;
        }

        Vehicle v       = parkedVehicles[plate];
        time_t exitTime = time(0);
        double hours    = difftime(exitTime, v.entryTime) / 3600.0;
        if (hours < 1) hours = 1;
        double fee = hours * getHourlyRate(v.type);

        Ticket t;
        t.ticketId    = nextTicketId++;
        t.plate       = plate;
        t.vehicleType = v.type;
        t.spotId      = v.spotId;
        t.entryTime   = v.entryTime;
        t.exitTime    = exitTime;
        t.fee         = fee;
        ticketHistory.push_back(t);
        totalRevenue += fee;

        for (int i = 0; i < (int)spots.size(); i++) {
            if (spots[i].id == v.spotId) {
                spots[i].isOccupied    = false;
                spots[i].occupantPlate = "";
                break;
            }
        }
        parkedVehicles.erase(plate);

        printLine();
        cout << "  [EXIT] Receipt  -  Ticket No: " << t.ticketId << endl;
        printLine();
        cout << "  Plate    : " << plate                             << endl;
        cout << "  Type     : " << getVehicleTypeName(t.vehicleType) << endl;
        cout << "  Spot No  : " << t.spotId                          << endl;
        cout << "  Entry    : " << formatTime(t.entryTime)           << endl;
        cout << "  Exit     : " << formatTime(t.exitTime)            << endl;
        cout << "  Duration : " << fixed << setprecision(2) << hours << " hour(s)" << endl;
        cout << "  Fee      : Rs." << fixed << setprecision(2) << fee << endl;
        printLine();
        return true;
    }

    void showAvailability() {
        clearScreen();
        printBoxedTitle(name + " - Parking Availability", 80);

        int sf = 0, st = 0, mf = 0, mt = 0, lf = 0, lt = 0;
        double total = spots.size();

        for (int i = 0; i < (int)spots.size(); i++) {
            if (spots[i].size == Small) {
                st++;
                if (!spots[i].isOccupied) sf++;
            } else if (spots[i].size == Medium) {
                mt++;
                if (!spots[i].isOccupied) mf++;
            } else {
                lt++;
                if (!spots[i].isOccupied) lf++;
            }
        }

        double occ = 1.0 - ((double)(st + mt + lt - sf - mf - lf) / total);

        drawParkingGrid(spots, parkedVehicles);

        cout << "\n 📊  SPOT SUMMARY" << endl;
        printLine('─', 80);
        setColor(BLUE); cout << " Overall Occupancy: "; resetColor();
        drawProgressBar(occ, 30, (occ > 0.8 ? RED : GREEN));

        cout << left << setw(12) << "\n Small: " << setw(8) << sf << "/" << st << "  ";
        setColor(GREEN); cout << fixed << setprecision(1) << (sf/(double)st*100) << "%"; resetColor(); cout << " free" << endl;
        cout << left << setw(12) << " Medium: " << setw(8) << mf << "/" << mt << "  ";
        setColor(GREEN); cout << fixed << setprecision(1) << (mf/(double)mt*100) << "%"; resetColor(); cout << " free" << endl;
        cout << left << setw(12) << " Large: " << setw(8) << lf << "/" << lt << "  ";
        setColor(GREEN); cout << fixed << setprecision(1) << (lf/(double)lt*100) << "%"; resetColor(); cout << " free" << endl;
        cout << " Total Free: " << (sf+mf+lf) << "/" << total << endl;
        printLine('─', 80);

        cout << "\nPress Enter to return...";
        cin.get();
    }

    void showParkedVehicles() {
        printLine('=');
        cout << "  Currently Parked Vehicles" << endl;
        printLine('=');

        if (parkedVehicles.empty()) {
            cout << "  No vehicles are currently parked." << endl;
        } else {
            cout << left
                 << "  " << setw(15) << "Plate"
                 << setw(10) << "Type"
                 << setw(8)  << "Spot"
                 << "Entry Time" << endl;
            printLine();
            for (map<string,Vehicle>::iterator it = parkedVehicles.begin();
                 it != parkedVehicles.end(); it++) {
                Vehicle& v = it->second;
                cout << "  " << left
                     << setw(15) << v.plate
                     << setw(10) << getVehicleTypeName(v.type)
                     << setw(8)  << v.spotId
                     << formatTime(v.entryTime) << endl;
            }
        }
        printLine('=');
    }

    void searchVehicle(string plate) {
        if (parkedVehicles.count(plate) == 0) {
            cout << "\n[SEARCH] Vehicle " << plate << " is not currently parked." << endl;
            bool found = false;
            for (int i = 0; i < (int)ticketHistory.size(); i++) {
                if (ticketHistory[i].plate == plate) {
                    cout << "  History: Ticket #" << ticketHistory[i].ticketId
                         << "  Exit: " << formatTime(ticketHistory[i].exitTime)
                         << "  Fee: Rs." << fixed << setprecision(2)
                         << ticketHistory[i].fee << endl;
                    found = true;
                }
            }
            if (!found) cout << "  No record found for plate: " << plate << endl;
            return;
        }

        Vehicle& v    = parkedVehicles[plate];
        time_t   now  = time(0);
        double   hours = difftime(now, v.entryTime) / 3600.0;
        if (hours < 1) hours = 1;
        double estFee = hours * getHourlyRate(v.type);

        printLine();
        cout << "  [FOUND] Vehicle Details" << endl;
        printLine();
        cout << "  Plate     : " << v.plate                      << endl;
        cout << "  Type      : " << getVehicleTypeName(v.type)   << endl;
        cout << "  Spot No   : " << v.spotId                     << endl;
        cout << "  Entry     : " << formatTime(v.entryTime)      << endl;
        cout << "  Parked for: " << fixed << setprecision(2) << hours << " hour(s)" << endl;
        cout << "  Est. Fee  : Rs." << fixed << setprecision(2) << estFee << endl;
        printLine();
    }

    void showReport() {
        printLine('=');
        cout << "  Revenue Report - " << name << endl;
        printLine('=');
        cout << "  Total vehicles served : " << totalServed           << endl;
        cout << "  Total transactions    : " << ticketHistory.size()  << endl;
        cout << "  Total revenue         : Rs." << fixed << setprecision(2)
             << totalRevenue << endl;

        if (!ticketHistory.empty()) {
            printLine();
            cout << "  Last 5 Transactions:" << endl;
            printLine();
            int start = (int)ticketHistory.size() - 5;
            if (start < 0) start = 0;
            for (int i = start; i < (int)ticketHistory.size(); i++) {
                Ticket& t = ticketHistory[i];
                cout << "  #" << setw(3) << t.ticketId
                     << "  " << left << setw(15) << t.plate
                     << setw(8) << getVehicleTypeName(t.vehicleType)
                     << "Rs." << fixed << setprecision(2) << t.fee << endl;
            }
        }
        printLine('=');
    }

    // New dashboard method
    void showDashboard() {
        clearScreen();
        printBoxedTitle("🅿️  UIC PARKING DASHBOARD 🚗", 80);
        
        // Calculate stats
        int totalSpots = spots.size();
        int freeSpots = 0;
        int smallFree = 0, medFree = 0, largeFree = 0;
        int smallTot = 0, medTot = 0, largeTot = 0;
        double occupancy = 1.0 - ((double)parkedVehicles.size() / totalSpots);
        
        for (const auto& spot : spots) {
            if (!spot.isOccupied) freeSpots++;
            if (spot.size == Small) { smallTot++; if (!spot.isOccupied) smallFree++; }
            else if (spot.size == Medium) { medTot++; if (!spot.isOccupied) medFree++; }
            else { largeTot++; if (!spot.isOccupied) largeFree++; }
        }
        
        // Parking Grid
        drawParkingGrid(spots, parkedVehicles);
        
        // Stats Panel
        cout << "\n 📊  LIVE STATS" << endl;
        printLine('─', 80);
        setColor(BLUE);
        cout << " Occupancy: "; resetColor();
        drawProgressBar(occupancy, 30, (occupancy > 0.8 ? RED : GREEN));
        
        cout << " Total Spots: " << totalSpots << " | Free: " << freeSpots << " (" << fixed << setprecision(0) << (100*(freeSpots/(double)totalSpots)) << "%)" << endl;
        cout << " Small: " << smallTot << "/" << smallFree << " | ";
        cout << " Med: " << medTot << "/" << medFree << " | ";
        cout << " Large: " << largeTot << "/" << largeFree << endl;
        cout << " Active Vehicles: " << parkedVehicles.size() << " | Revenue: Rs." << fixed << setprecision(2) << totalRevenue << endl;
        printLine('─', 80);
        
        // Recent Activity (last 3 parked - simplistic)
        cout << "\n 📋  RECENT ACTIVITY (Last 3)" << endl;
        printLine('-', 80);
        int count = 0;
        for (auto it = parkedVehicles.rbegin(); it != parkedVehicles.rend() && count < 3; ++it, ++count) {
            const Vehicle& v = it->second;
            double hours = difftime(time(0), v.entryTime) / 3600.0;
            setColor(GREEN); cout << " ➕ "; resetColor();
            cout << v.plate << " (" << getVehicleTypeName(v.type) << ") Spot " << v.spotId
                 << " | " << fixed << setprecision(1) << hours << "hrs | " << formatTime(v.entryTime) << endl;
        }
        if (parkedVehicles.empty()) cout << " No recent activity." << endl;
        
        // Quick Menu
        printLine('═');
        cout << "\n 🎛️️  QUICK ACTIONS (Press 0-7 or Enter to refresh)" << endl;
        cout << " 1:Park  2:Exit  3:Availability  4:Parked  5:Search  6:Report" << endl;
        cout << " 7:Back  0:Quit" << endl;
        printLine('═');
        
        cout << "\nPress Enter to refresh...";
        cin.ignore();
        cin.get();  // Wait for keypress
    }
};

// ============================================================
//  MENU FUNCTIONS
// ============================================================

void showMenu() {
    cout << "\n+----------------------------------+" << endl;
    cout << "|   Smart Parking Management       |" << endl;
    cout << "+----------------------------------+" << endl;
    cout << "| 1. Park a vehicle                |" << endl;
    cout << "| 2. Exit a vehicle                |" << endl;
    cout << "| 3. View availability             |" << endl;
    cout << "| 4. View all parked vehicles      |" << endl;
    cout << "| 5. Search vehicle by plate       |" << endl;
    cout << "| 6. Revenue report                |" << endl;
    cout << "| 0. Quit                          |" << endl;
    cout << "+----------------------------------+" << endl;
    cout << "Enter choice: ";
}

VehicleType selectVehicleType() {
    cout << "\nSelect vehicle type:" << endl;
    cout << "  1. Car   (Rs.20/hr)" << endl;
    cout << "  2. Bike  (Rs.10/hr)" << endl;
    cout << "  3. Truck (Rs.40/hr)" << endl;
    cout << "Enter choice: ";
    int choice;
    cin >> choice;
    if (choice == 2) return Bike;
    if (choice == 3) return Truck;
    return Car;
}

// ============================================================
//  MAIN
// ============================================================

int main() {

    ParkingLot lot("UIC  Parking", 5, 5, 3);

    cout << "\n--- Demo: Parking 3 sample vehicles ---" << endl;
    lot.parkVehicle("MH01AB1234", Car);
    lot.parkVehicle("DL02CD5678", Bike);
    lot.parkVehicle("KA03EF9012", Truck);

    int choice;

    do {
        showMenu();
        cin >> choice;
        cin.ignore();

        if (choice == 1) {
            string plate;
            cout << "Enter plate number: ";
            getline(cin, plate);
            for (int i = 0; i < (int)plate.length(); i++) plate[i] = toupper(plate[i]);
            VehicleType type = selectVehicleType();
            lot.parkVehicle(plate, type);

        } else if (choice == 2) {
            string plate;
            cout << "Enter plate number: ";
            getline(cin, plate);
            for (int i = 0; i < (int)plate.length(); i++) plate[i] = toupper(plate[i]);
            lot.exitVehicle(plate);

        } else if (choice == 3) {
            lot.showAvailability();

        } else if (choice == 4) {
            lot.showParkedVehicles();

        } else if (choice == 5) {
            string plate;
            cout << "Enter plate number: ";
            getline(cin, plate);
            for (int i = 0; i < (int)plate.length(); i++) plate[i] = toupper(plate[i]);
            lot.searchVehicle(plate);

        } else if (choice == 6) {
            lot.showReport();

        } else if (choice == 0) {
            cout << "\nThank you for using Smart Parking. Goodbye!" << endl;

        } else {
            cout << "Invalid choice. Please try again." << endl;
        }

    } while (choice != 0);

    return 0;
}