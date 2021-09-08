#include <iostream>
#include <sstream>
#include "openssl/sha.h"
#include <iomanip>
#include <vector>
#include <fstream>
#include <ncurses.h>

std::string sha256 (const std::string str, const std::string spice) {
	const std::string toHash = str+spice;
	unsigned char hash[SHA256_DIGEST_LENGTH];
	SHA256_CTX sha256;
	SHA256_Init(&sha256);
	SHA256_Update(&sha256, toHash.c_str(), toHash.size());
	SHA256_Final(hash, &sha256);
	std::stringstream ss;
	for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
		ss << std::hex << std::setw(2) << std::setfill('0') << int(hash[i]);
	}
	return ss.str();
}

std::vector<std::string> addAppID(std::string input, std::vector<std::string> applications){
	applications.push_back(input);
	return applications;
}

std::string input() {
	std::string x;
	std::cin >> x;
	return x;
}

void saveToFile(std::string applications) {
	std::ofstream save;
	save.open("Applications", std::ios_base::app);
	save << "\n" << applications;
	save.close();
}

std::vector<std::string> getData() {
	std::vector<std::string> data;

	std::ifstream read("Applications");

	std::string str;
	while(std::getline(read, str)) {
		data.push_back(str);
	}

	return data;
}

//curses function
//SCARY FUNCTION
std::string getstring(WINDOW * win, bool secret)
{
    std::string input;

    noecho();
    // let the terminal do the line editing

    // this reads from buffer after <ENTER>, not "raw" 
    // so any backspacing etc. has already been taken care of
    int ch = wgetch(win);

    int i = 0;

    while ( ch != '\n' )
    {
    	if(ch == 8) {
    		input.pop_back();
    	} else {
    		input.push_back( ch );
        	ch = wgetch(win);
        		if(secret) {
        		wclear(win);
        		box(win, 0, 0);
				wmove(win, 1, 1);
				wprintw(win, "Enter passphrase");
        		refresh();
				wrefresh(win);
				wmove(win, 2, 1+i);
				char c = char(ch);
				char *x = &c;
				wprintw(win, x);
        	}
    	}
        i++;
    }
    if(secret) {
    	wrefresh(win);
    	box(win, 0, 0);
		refresh();
		wrefresh(win);
		wmove(win, 2, 1);
    }
    return input;
}

std::string passwordInput(WINDOW * win, int y, std::string selection) {
	wmove(win, 1, 1);
	wprintw(win, "Enter passphrase");
	refresh();
	wrefresh(win);
	wmove(win, 2, 1);

	std::string password = getstring(win, true);
	std::string combined = password + selection;
	move(y-7, 2);

	return sha256(password, selection);
}

void saveNewApplication(WINDOW * win) {
	wmove(win, 1, 1);
	wprintw(win, "Enter Application name");
	wmove(win, 2, 1);
	wrefresh(win);
	saveToFile(getstring(win, false));
}

const char * menuScreen(WINDOW * win, std::vector<std::string> applications) {
	keypad(win, true);

	int choice;
	int highlight = 0;

	applications.push_back("Add new app...");

	while(true) {
		for(int i = 0; i < applications.size(); i++) {
			if(i == highlight) {
				wattron(win, A_REVERSE);
			}
			mvwprintw(win, i+1, 1, applications[i].c_str());
			wattroff(win, A_REVERSE);
		}
		choice = wgetch(win);

		switch(choice){
			case KEY_UP:
				highlight--;
				break;
			case KEY_DOWN:
				highlight++;
				break;
			default:
				break;

		}
		if (choice == 10) {
			break;
		}
	}

	if (highlight == applications.size()-1) {
		return "inputApp";
	}
	applications.pop_back();

	return applications[highlight].c_str();
}

int main() {
	std::vector<std::string> applications = getData();

	//NCURSES
	initscr();

	//CREATING WINDOWS

	int height, width, start_y, start_x;
	width = 20;

	int y, x;
	getmaxyx(stdscr, y, x);

	int yMax, xMax;
	getmaxyx(stdscr, yMax, xMax);

	WINDOW * window = newwin((y/2)+applications.size(), x, 0, 0);
	box(window, 0, 0);
	refresh();
	wrefresh(window);

	WINDOW * passwdInput = newwin(4, xMax/2, y-6, 1);
	box(passwdInput, 0, 0);

	while(true) {
		height = applications.size()+3;
		
		WINDOW * win = newwin(height+1, width, 1, 1);
		box(win, 0, 0);
		refresh();
		wrefresh(win);

		std::string selection = menuScreen(win, applications);

		if (selection == "inputApp") {
			saveNewApplication(passwdInput);

			int i, j;
			applications = getData();
			wclear(passwdInput);
        	box(passwdInput, 0, 0);
        	wmove(passwdInput, 1, 1);
			wprintw(passwdInput, "Enter Application name");
			wrefresh(passwdInput);
			wrefresh(win);
		} else {
			printw((passwordInput(passwdInput, y, selection)).c_str());
			wrefresh(passwdInput);
		}
	}
	endwin();
}