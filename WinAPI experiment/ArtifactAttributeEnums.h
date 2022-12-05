#pragma once
namespace rects {
	class TextBox {
		using ushort = unsigned short;
	public:
		ushort posX = 0;
		ushort posY = 0;
		ushort width = 0;
		ushort height = 0;
	};

	TextBox mainStatKey{ 1111, 228, 252, 20 };
	TextBox mainStatValue{ 1111, 252, 200, 33 };
	TextBox slotKey{ 1111, 159, 192, 18 };
	TextBox setKey{ 1111, 530, 375,20 };

	TextBox* boxes[] = { &mainStatKey, &mainStatValue, &slotKey, &setKey };
}

