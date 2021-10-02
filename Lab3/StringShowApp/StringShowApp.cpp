#include <iostream>
#include <Windows.h>

int main()
{
	bool isExit = false;

	char src[255] = { 0 };

	while (!isExit)
	{
		std::cout << "1 - Set string\n2 - Show string\n3 - Get PID\n4 - Exit\n";

		char action;

		std::cin >> action;

		switch (action)
		{
		case '1':
			std::cout << "Source string: ";
			std::cin >> src;
			break;
		case '2':
			std::cout << "String: " << src << std::endl;
			break;
		case '3':
			std::cout << "PID: " << GetCurrentProcessId() << std::endl;
			break;
		case '4':
			isExit = true;
			break;
		default:
			break;
		}
	}
}
