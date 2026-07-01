#include<iostream>
#include<iomanip>
#include<pqxx/pqxx>
#include<memory>
#include<list>
#include<vector>
#include<string>
#include <windows.h>
using namespace std;
class Task {
public:
	size_t id;
	static int id_count;
	string tasktext;
	Task(size_t idd, string n) :id(idd), tasktext(n) {
	}
	~Task() {}
};
class TaskManager {
private:
	pqxx::connection& conn;
public:
	size_t id;
	vector<unique_ptr<Task>>spisok;
	TaskManager(pqxx::connection& con) : conn(con) {
		pqxx::work tx(conn);
		string getid = "SELECT id,task FROM to_do_list ORDER BY id ;";
		pqxx::result res = tx.exec(getid);
		for (auto it : res) {
			size_t db_id = it[0].as<size_t>();
			string db_task = it[1].as<string>();
			spisok.push_back(make_unique<Task>(db_id, db_task));//Запись того что уже лежит в бд
		}
	}
	void addtask(const string& taskname) {
		pqxx::work tx(conn);
		string add_table = "INSERT INTO to_do_list(task)VALUES (" + tx.quote(taskname) + ") RETURNING id;";
		pqxx::result res = tx.exec(add_table);
		size_t gen_id = res[0][0].as<size_t>();
		spisok.push_back(make_unique<Task>(gen_id, taskname));
		tx.commit();
		cout << "Задача записана" << endl;
	}
	void showtask() {
		if (spisok.empty()) {
			cout << "Пока нет добавленных задач" << endl;
		}
		else {
			for (const auto& i : spisok) {
				cout << "Задача №" << i->id << ": " << i->tasktext << endl;
			}
		}
	}
	void taskremove(auto& index) {
		if (spisok.empty()) {
			cout << "Список задач пока пуст =(" << endl;
			return;
		}
		else {
			for (size_t i = 0; i < spisok.size(); i++) {
				if (spisok[i]->id == index) {
					spisok.erase(spisok.begin() + i);
					pqxx::work tx(conn);
					string deltab = "DELETE FROM to_do_list WHERE id= " + to_string(index) + ";";
					tx.exec(deltab);
					tx.commit();
					cout << "Задача удалена" << endl;
					return;
				}
			}
		}
	}
	~TaskManager() {}
};
int main() {
	SetConsoleCP(65001);
	SetConsoleOutputCP(65001);
	bool work = true;
	int choice = 0;
	pqxx::connection connect("dbname=To_Do_List user=postgres password=1234 host=localhost port=5432");
	pqxx::work tx(connect);
	string query_table = "CREATE TABLE IF NOT EXISTS to_do_list(id SERIAL PRIMARY KEY, task VARCHAR(500) NOT NULL);";
	tx.exec(query_table);
	tx.commit();
	TaskManager manager(connect);
	cout << setw(10) << "Welcome to To Do List| Добро пожаловать!" << setw(10) << endl;
	while (work == true) {
		cout << "Press: 1-добавить, 2-удалить задачу, 0-выход" << endl;
		cin >> choice;
		if (choice == 1) {
			string ptask;
			cout << "Какова ваша цель?" << endl;
			cin >> ws;
			getline(cin, ptask);
			manager.addtask(ptask);
			cout << "Список ваших задач: " << endl;
			manager.showtask();
		}
		else if (choice == 2) {
			size_t deltask;
			cout << "Какую задачу желате удалить?" << endl;
			cin >> deltask;
			manager.taskremove(deltask);
		}
		else if (choice == 0) {
			work = false;
		}
	}
	system("Pause");
	return 0;
}