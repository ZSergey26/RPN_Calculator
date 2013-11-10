// ExprParser.cpp: определяет точку входа для консольного приложения.
//

#include "stdafx.h"

#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <sstream>
#include <math.h>

#define DEBUG true // Выводить отладочную информацию?

using namespace std;

// Функция рассчета обратной польской записи
long double calcRPN(vector<string> inputStr);


// Вывод стека на экран
void printStack(vector<string> st);
void printStack(vector<long double> st);

// Функции преобразования типов
string charToString(char c);

double strToDouble(string s);


class ParsedNum
{	
	public:
		string num;
		int pos;

};

class ParsedWord
{
	public:
		string word;
		int pos;
};

enum Operation
{
	ADD,
	SUB,
	MUL,
	DIV, 
	FUNC
} ;


// Таблица допустимых имен
const unsigned int NAME_COUNT = 5;
static const string NameTable[NAME_COUNT] = {"pow", "exp", "abs", "sin", "cos"};

// Функции разбора строки
ParsedNum  parseNum(string inputStr, int pos);
ParsedWord parseWord(string inputStr, int pos);

// Разбор унарных операций
void parseUnaryOperation(string &str);

// Проверка на неправиоьный порядок операторов
bool wrongOperation(string str);

// Поиск имени в таблице имен
bool searchName(string str);

void parseComma(string str);

// Проверка является ли символ оператором
bool isOperator(char c);
bool isOperator(string s);

// Приоритет операций
Operation getOperation(string s);
int getPriority(Operation oper);
bool priorityBiggerThan(string s1, string s2);


string parseSub(string inputStr);

void parseBracket(string &str);



int _tmain(int argc, _TCHAR* argv[])
{
	// Ввод строки
	string inputExpr;
	cout   << "Enter expression: ";
	getline(cin, inputExpr);

	parseBracket(inputExpr);
	// Удаление пробелов
	inputExpr.erase( remove_if(inputExpr.begin(), inputExpr.end(), ::isspace),  inputExpr.end() );
	parseComma(inputExpr);
	inputExpr = parseSub(inputExpr);
	cout << "End parse sub" << endl;
	parseUnaryOperation(inputExpr);
	


	if(DEBUG)
	{
		cout << "Unary operation parsed: " << inputExpr << endl << endl;
	}

	if (wrongOperation(inputExpr))
	{
		cout << "Error: incorrect sequence of operators" << endl;
		system("pause");
		return 1;
	}

	// Выходная строка в обратной польской записи
	vector<string> outString;

	// Стек
	vector<string> Stack;
	
	// Просматриваем строку слева направо
	for(int i = 0; i < inputExpr.length(); i++)
	{
		if(i >= inputExpr.length())
			break;

		if(DEBUG)
		{	
			cout << "Step: " << i << endl;
			cout << "Current Stack size: " << Stack.size() << endl;
			cout << "Stack: ";
			printStack(Stack);
			cout << "Output string: ";
			printStack(outString);
			cout << "ACTION: " << endl;
	
		}

		// Если символ является числом, добавляем его к выходной строке.
		if ( isdigit(inputExpr[i]) )
		{
			
			ParsedNum buf;
			buf = parseNum(inputExpr, i);

			if (i >= 1)
				if(inputExpr[i-1] == '~')
					buf.num = "-" + buf.num;


			if(DEBUG)
			{
				cout << "Put number '" << buf.num << "' into output string" << endl << endl;
			}

			outString.push_back(buf.num);
			i = buf.pos;
			
			
		}


		// Если токен — функция, то поместить его в стек
		if ( isalpha(inputExpr[i]) )
		{
			ParsedWord buf;
			buf = parseWord(inputExpr, i);


			// Проверим, содерржится ли считанное слово в таблице
			if ( !searchName(buf.word) )
			{
				cout << "Error: expression contains invalid characters - " << buf.word << endl;
				system("pause");
				exit(1);
			}

			Stack.push_back(buf.word);
			i = buf.pos;
			if(DEBUG)
			{
				cout << "Put function '" << buf.word << "' in stack" << endl << endl;
			}
			
		}

		// Если символ - запятая (разделитель аргументов)
		if ( inputExpr[i] == ',' )
		{
			if(DEBUG)
			{
				cout << "Get comma"<< endl;
			}
			// Пока на вершине стека не открывающая скобка
			for (int i = Stack.size() - 1; i >= 0 ; i--)
			{
						
				// Перекладываем операторы из стека в выходную очередь
				if (Stack[i] != "(")
				{
					if(DEBUG)
					{
						cout << "Push operator "<< Stack[i] << " in to output string" << endl;
					}
					outString.push_back(Stack[i]);
					Stack.pop_back();
				}
				else 
				{
					if(DEBUG)
					{
						cout << "Now we met (so exit the loop " << endl << endl;
					}
					
					break;
				}
			}
		} 

		// Если символ является оператором:
		if ( isOperator(inputExpr[i]) )
		{
			
			string op1 = charToString(inputExpr[i]);

			if(DEBUG)
			{
				cout << "Get operation '" << op1 << "'" << endl ;
			}
			
			if ( !Stack.empty() )
			{
				

				if(DEBUG)
				{
					cout << "Stack is't empty " << endl ;
				}

				// Пока на вершине стека оператор
				string op2 = Stack[Stack.size() - 1];
				for (int i = Stack.size() - 1; (i >= 0) && (searchName(op2) || isOperator(op2))  ; --i)
				{
					
					if(DEBUG)
					{
						cout << "In loop now... " <<  endl ;
						cout << "Iter: " << i << endl;
						cout << "Compare " << op2 << " with " << op1 <<endl;
					}

					// Если приоритет у оператора на вершине стека меньше либо
					// равен приоритету считанного из строки оператора
					if ( !priorityBiggerThan(op1, op2) && isOperator(op2) )
					{
						if(DEBUG)
						{
							cout << "Priority of " << op1 << " less or equally then " << op2 << endl ;
							cout << "Put operation " << op2 << " in to output string" << endl << endl;
						}

						// Тогда выталкиваем операцию из стека в выходную строку
						outString.push_back(op2);
						Stack.pop_back();
					}

					// Если на вершине функция, то мы должны вытолкнуть ее в итоговую строку в любом случае
					// так как она имеет наибольший приоритет
					if (searchName(op2) )
					{
						if(DEBUG)
						{
							cout << "Priority of " << op1 << " less or equally then function " << op2 << endl ;
							cout << "Put function " << op2 << " in to output string" << endl << endl;
						}

						outString.push_back(op2);
						Stack.pop_back();
					}

					if (!Stack.empty())
					{		
						op2 = Stack[Stack.size() - 1];						
					}
				}

				if(DEBUG)
				{
					cout << "Put operation " << op1 << " in to stack" << endl << endl;
				}
				Stack.push_back(op1);

			}
			else
			{
				Stack.push_back(op1);
				if(DEBUG)
				{
					cout << "As stack is empty put operation in to stack" << endl << endl;
				}
			}
		}


		// Если символ — открывающая скобка, то положить его в стек.
		if (inputExpr[i] == '(')
		{
			if(DEBUG)
			{
					cout << "Put '(' in to stack" << endl << endl;
			}

			string buf = charToString(inputExpr[i]);
			Stack.push_back(buf);
		}

		// Если символ — закрывающая скобка:
		if (inputExpr[i] == ')' )
		{

			// Если парной скобки нет - значит ввели лишнюю закрывающую скобку
			if (Stack.empty() )
			{
				cout << "Error: incorrect placement of the brackets " << endl;
				system("pause");
				return 1;
			}


			// Ищем парную открывающую скобку и выталкиваем все встречающиеся операции из стека в выходную строку по пути
			for (int i = Stack.size() - 1; Stack[i] != "(" && i > 0; i--)
			{
				outString.push_back(Stack[i]);
				Stack.pop_back();
			}
			Stack.pop_back();
		}


	} // Основной for


	// 
	if (!Stack.empty())
		for (int i = Stack.size() - 1; i >= 0 ; i--)
		{
			if (Stack[i] == "(")
			{
				cout << "Error: incorrect placement of the brackets " << endl;
				system("pause");
				return 1;
			}
			outString.push_back(Stack[i]);
		}

	cout <<endl << "Output string in reverse polish notation: " ;
	printStack(outString);
	cout << endl;
	if (!outString.empty())
		cout << "Expression value = " << calcRPN(outString) << endl;
	else
	{
		cout << "Error: can't calc expresion " << endl;
		system("pause");
		return 1;
	}
	system("pause");
	return 0;
}


// Выводим содержимое стека на экран
void printStack(vector<string> st)
{

	if(st.empty())
	{
		cout << "is empty!" << endl;
		return;
	}

	for(int i = 0; i < st.size(); i++)
		cout << st[i] << " ";
	cout << endl;
}

void printStack(vector<long double> st)
{

	if(st.empty())
	{
		cout << "is empty!" << endl;
		return;
	}

	for(int i = 0; i < st.size(); i++)
		cout << st[i] << " ";
	cout << endl;
}


// Перевод из символа в строку
string charToString(char c)
{
	stringstream ss;
	string s;

	ss << c;
	ss >> s;

	return s;
}


// Вытаскиваем число из стека
ParsedNum parseNum(string inputStr, int pos)
{
	ParsedNum Number;

	while(isdigit(inputStr[pos]) || inputStr[pos] == '.' )
	{
		
		Number.num = Number.num + charToString(inputStr[pos]); 
		pos++;

		if(pos >= inputStr.length())
			break;
	}

	Number.pos = pos - 1;
	return Number;
}


// Вытаскиваем слово из строки
ParsedWord parseWord(string inputStr, int pos)
{
	ParsedWord Word;

	while( isalpha(inputStr[pos]) )
	{
		Word.word = Word.word + charToString(inputStr[pos]); 
		pos++;

		if(pos >= inputStr.length())
			break;
	}

	Word.pos = pos - 1;
	return Word;
}

// Проверяем является ли символ оператором
bool isOperator(char c)
{
	if (c == '+' || c == '-' || c == '*' || c == '/')
		return true;
	else 
		return false;
}

bool isOperator(string s)
{
	if (s == "+" || s == "-" || s == "*" ||s == "/")
		return true;
	else 
		return false;
}


Operation getOperation(string s)
{
	if (s == "+")
		return ADD;
	if (s == "-")
		return SUB;
	if (s == "*")
		return MUL;
	if (s == "/")
		return DIV;
	if ( searchName(s))
		return FUNC;
}

int getPriority(Operation oper)
{
	switch(oper)
	{
		case ADD  : return 1;
		case SUB  : return 1;
		case MUL  : return 2;	
		case DIV  : return 2;
		case FUNC : return 3;
	}

}

// Приоритет первого аргумента больше?
bool priorityBiggerThan(string s1, string s2)
{
	Operation o1 = getOperation(s1);
	Operation o2 = getOperation(s2);

	if ( getPriority(o1) > getPriority(o2) )
		return true;
	else
		return false;
}

bool searchName(string str)
{

	for(int i = 0; i < NAME_COUNT; i++)
	{
		if(str == NameTable[i])
		{
			return true;
		}
	}

	return false;
}


// Разбираемся с унарными операциями
void parseUnaryOperation(string &str)
{

	// Проходим по всей строке
	for(int i = 0; i < str.length() - 1; i++)
	{
		// Если встречаем оператор, то...
		if( isOperator(str[i]) )
		{
			// Если оператор стоит первым символом в строке например -2*3
			if(i == 0)
			{
				switch ( getOperation( charToString(str[i]) ) )
				{

					// Если плюс - удаляем символ
					case ADD : 
					{
						str.erase(0,1);
						if(DEBUG)
						{
							cout << "Delete first + operator " << endl;
						}
						break;
					}

					// Минус меняем на ~, чтобы потом занести число с минусом в стек
					case SUB : 
					{

						if (str[i + 1] == '(')
						{
							str.erase(0, 1);
							str += "*~1";
						}
						
						else
							str[i] = '~';

						if(DEBUG)
						{
							cout << "Replace '-' as '~' " << endl;
						}

						break;
					}

					// Умножение
					case MUL : 
					{
						cout << "Error: invalid expression. * can't be first operator " << endl;
						system("pause");
						exit(1);
					}

					case DIV : 
			    	{
						cout << "Error: invalid expression. / can't be first operator " << endl;
						system("pause");
						exit(1);
					}
				}
			}

			// Все остальные случаи
			else
			{
				if (str[i] == '-' && str[i + 1] == '(')
				{
					str.erase(i, 1);
					str += "*~1";

				}

				// Является ли оператор унарным
				if ( ( isOperator(str[i-1]) || str[i-1] == '(' || str[i-1] == ',') && ( isdigit(str[i+1]) ) )
				{
					switch ( getOperation( charToString(str[i]) ) )
					{

						// Если плюс - удаляем символ
						case ADD : 
						{
							str.erase(i,1);

							if(DEBUG)
							{
								cout << "Delete + operator at " << i << " symbol" << endl;
							}

							break;
						}

						// Минус меняем на ~, чтобы потом занести число с минусом в стек
						case SUB : 
						{
							str[i] = '~';

							if(DEBUG)
							{
								cout << "Replace '-' as '~'  at " << i << " symbol" << endl;
							}

							break;
						}

						// Умножение
						case MUL : 
						{
							cout << "Error: invalid expression. * can't be first operator " << endl;
							system("pause");
							exit(1);
						}

						case DIV : 
			    		{
							cout << "Error: invalid expression. / can't be first operator " << endl;
							system("pause");
							exit(1);
						}
					}
				}
			}
		}
	}
	
}


bool wrongOperation(string str)
{
	for(int i = 0; i < str.length() - 1; i++)
	{
		char curSym  = str[i];
		char nextSym = str[i+1];

		// Если подряд идут ** или */
		if(curSym == '*' && (nextSym == '*' || nextSym == '/'))
			return true;

		if(curSym == '/' && (nextSym == '*' || nextSym == '/'))
			return true;

		if(curSym == '.' && (nextSym == '.' || nextSym == ','))
			return true;

		if(curSym == ',' && (nextSym == '.' || nextSym == ','))
			return true;

	}


// Последний символ не должен быть оператором
	if (isOperator (str[str.length() - 1]) )
		return true;
	return false;

}


// Функция рассчета обратной польской записи
long double calcRPN(vector<string> inputStr)
{
	if(DEBUG)
	{
		cout << "Now calc expression in reverse polish notation " << endl;
	}

	long double calcExpression;
	vector<long double> Stack;

	for(int i = 0; i < inputStr.size(); i++)
	{
		if(DEBUG)
		{
			cout << "Step " << i << endl;
		}

		//Если элемент входной строки оператор либо функция
		if(  isOperator(inputStr[i]) || searchName(inputStr[i]) )
		{
			long double d;

			
			// Если считали операцию, а в стеке недостаточно аргументов, то выводим ошибку
			if(Stack.size() < 2)
			{
				cout << "Error: input expressin incorrect! " << endl;
				system("pause");
				exit(0);
			}

			if(DEBUG)
			{
				cout << "Get operation or func " << inputStr[i] << endl;
			}

			//Смотрим что за операция и применяем ее к двум верхним элементам стека
			switch ( getOperation(inputStr[i]) )
			{
				case ADD  : 
				{
					if(DEBUG)
					{
						cout << "Do addition: " << Stack[Stack.size() - 2] << " and " << Stack[Stack.size() - 1]  << endl;
					}

					d = Stack[Stack.size() - 2] + Stack[Stack.size() - 1];
					Stack.pop_back();
					Stack.pop_back();

					Stack.push_back(d);				
					if(DEBUG)
					{
						cout << "Put " << d << " on stack"  << endl ;
						cout << "Stack: ";
						printStack(Stack);
						cout << endl << endl;
					}

					break;

				}

				case SUB  : 
				{
					if(DEBUG)
					{
						cout << "Do subtraction: " << Stack[Stack.size() - 2] << " and " << Stack[Stack.size() - 1]  << endl;
					}

					d = Stack[Stack.size() - 2] - Stack[Stack.size() - 1];
					Stack.pop_back();
					Stack.pop_back();

					Stack.push_back(d);
					if(DEBUG)
					{
						cout << "Put " << d << " on stack"  << endl ;
						cout << "Stack: ";
						printStack(Stack);
						cout << endl << endl;
					}
					
					break;
				}

				case MUL  : 
				{
					if(DEBUG)
					{
						cout << "Do multiplication: " << Stack[Stack.size() - 2] << " and " << Stack[Stack.size() - 1]  << endl;
					}

					d = Stack[Stack.size() - 2] * Stack[Stack.size() - 1];
					Stack.pop_back();
					Stack.pop_back();

					Stack.push_back(d);	
					if(DEBUG)
					{
						cout << "Put " << d << " on stack"  << endl ;
						cout << "Stack: ";
						printStack(Stack);
						cout << endl << endl;
					}
					
					break;
				}

				case DIV  : 
				{
					if (Stack[Stack.size() - 1] != 0)
					{
						if(DEBUG)
						{
							cout << "Do division: " << Stack[Stack.size() - 2] << " and " << Stack[Stack.size() - 1]  << endl;
						}
						d = Stack[Stack.size() - 2] / Stack[Stack.size() - 1];
					}
					else
					{
						cout << "Error: division by zero" << endl;
						system("pause");
						exit(0);
					}

					Stack.pop_back();
					Stack.pop_back();

					Stack.push_back(d);
					if(DEBUG)
					{
						cout << "Put " << d << " on stack"  << endl ;
						cout << "Stack: ";
						printStack(Stack);
						cout << endl << endl;
					}
					
					break;
				}

				case FUNC : 
				{
					if(DEBUG)
					{
						cout << "Do func pow with args: " << Stack[Stack.size() - 2] << " and " << Stack[Stack.size() - 1]  << endl;
					}
					d = pow( Stack[Stack.size() - 2], Stack[Stack.size() - 1] );
					Stack.pop_back();
					Stack.pop_back();

					Stack.push_back(d);
					if(DEBUG)
					{
						cout << "Put " << d << " on stack"  << endl ;
						cout << "Stack: ";
						printStack(Stack);
						cout << endl << endl;
					}
					
					break;
				}
			
			}

		}
		else // Иначе на входе у нас число, просто ложим его в стек.
		{

			long double d = strToDouble(inputStr[i]);	
			Stack.push_back(d);

			if(DEBUG)
			{
				cout << "Get number " << inputStr[i] << " and put in to stack"  << endl;
				cout << "Stack: ";
				printStack(Stack);
				cout << endl << endl;
			}
		}
	}

	// Если в конце-концов в стеке что-нибудь осталось, значит выражение было неверным
	if(Stack.size() > 1)
	{
		cout << "Error: input expressin incorrect! " << endl;
		system("pause");
		exit(0);
	}
	else
		calcExpression = Stack[0];

	return calcExpression;
}


double strToDouble(string s)
{
	stringstream ss(s);

	double d;
	ss >> d;
	
	return d;
}

string parseSub(string inputStr)
{
	vector<string> Stack;

	
	for(int i = 0; i < inputStr.length(); i++)
	{
		if ( !Stack.empty() ) 
		{
			if( (inputStr[i] =='-') )
			{
				if( (Stack[Stack.size()-1] == "+"))
				{
					Stack.pop_back();
					Stack.push_back("-");
				}
				else
				{

					if( (Stack[Stack.size()-1] == "-"))
					{
						Stack.pop_back();
						Stack.push_back("+");
					}
					else
						Stack.push_back(charToString(inputStr[i]));
				}
			//Stack.push_back(charToString(inputStr[i]));	
			int x = 666;
			}
			else
			{
				//cout << "Put in stack" << endl;
				Stack.push_back(charToString(inputStr[i]));
			}
		}

		else
			Stack.push_back(charToString(inputStr[i]));
	}
	
	string out = "";
	for (int i = 0; i < Stack.size(); i++)
		out = out + Stack[i];
	return out;
}


void parseComma(string str)
{
	if(str[0] == '.' || str[str.length()-1] == '.' )
	{
		cout << "Bad comma" << endl;
		system("pause");
	    exit(0);
	}
	for(int i =0; i < str.length()-1; i++)
	{
		if( str[i] == '.')
			if( !isdigit(str[i-1]) || !isdigit(str[i+1]))
			{
				cout << "Bad comma" << endl;
				system("pause");
				exit(0);
			}
	}
}


void parseBracket(string &str)
{
	for(int i = 0; i < str.length()-1 ; i++)
	{
		if(str[i] == '(')
			if(str[i+1] == ')')
			{
				str.erase(i,2);
				str.insert(i,"0");
			}
	}
}
