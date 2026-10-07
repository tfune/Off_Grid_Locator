#pragma once

enum Member {
    SEBASTIAN,
    PROFESSOR_SALEMI
};

bool displayInit();
void displayStartup();

void displayMemberList(Member selectedMember);
void displayTracking(Member selectedMember, float distance, float direction);
void displayLocationUnavailable();
void displayInitializationError();
void displayRefreshBattery();