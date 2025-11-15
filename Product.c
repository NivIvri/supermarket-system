#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "Product.h"
#include "General.h"
#include "FileHelper.h"

#define MIN_DIG 3
#define MAX_DIG 5
typedef unsigned char BYTE;



static const char* typeStr[eNofProductType] = { "Fruit Vegtable", "Fridge", "Frozen", "Shelf" };
static const char* typePrefix[eNofProductType] = { "FV", "FR", "FZ", "SH" };


void	initProduct(Product* pProduct)
{
	initProductNoBarcode(pProduct);
	generateBarcode(pProduct);
}

void	initProductNoBarcode(Product* pProduct)
{
	initProductName(pProduct);
	pProduct->type = getProductType();
	initDate(&pProduct->expiryDate);
	pProduct->price = getPositiveFloat("Enter product price\t");
	pProduct->count = getPositiveInt("Enter product number of items\t");
}

void initProductName(Product* pProduct)
{
	do {
		printf("enter product name up to %d chars\n", NAME_LENGTH);
		myGets(pProduct->name, sizeof(pProduct->name), stdin);
	} while (checkEmptyString(pProduct->name));
}
int		saveProductToFile(const Product* pProduct, FILE* fp)
{
	if (fwrite(pProduct, sizeof(Product), 1, fp) != 1)
	{
		puts("Error saving product to file\n");
		return 0;
	}
	return 1;
}


void splitNumber(float num, int* integerPart, int* fractionalPart) {
	*integerPart = (int)num;
	*fractionalPart = (int)((num - (int)num) * 100);
}


int	saveProductToCompressedFile(const Product* pProduct, FILE* fp)
{
	BYTE data[4] = { 0 };

	int len = (int)strlen(pProduct->name);
	data[0] = pProduct->barcode[2] << 4 | pProduct->barcode[3];
	data[1] = pProduct->barcode[4] << 4 | pProduct->barcode[5];
	data[2] = pProduct->barcode[5] << 4;
	char* typeSubStr = (char*)malloc(PREFIX_LENGTH + 1);

	strncpy(typeSubStr, pProduct->barcode, PREFIX_LENGTH);

	for (int i = 0; i < eNofProductType; i++)
	{
		if (strcmp(typeSubStr, typePrefix[i]) == 0)
		{
			data[2] = i << 2 | data[2];
		}
	}
	data[2] = len >> 2 | data[2];
	data[3] = (len & 0x3) << 6;

	if (fwrite(&data, sizeof(BYTE), 4, fp) != 4)
		return 0;


	if (fwrite(pProduct->name, sizeof(char), len, fp) != len)
		return 0;

	BYTE data2[3] = { 0 };
	data2[0] = pProduct->count;
	int whole = (int)pProduct->price;
	int fraction = (int)((pProduct->price) * 100) % 100;

	data2[1] = fraction << 1 | (whole >> 8 & 0x1);
	data2[2] = (whole & 0xFF);

	if (fwrite(&data2, sizeof(BYTE), 3, fp) != 3)
		return 0;
	int year = pProduct->expiryDate.year-2024;
	BYTE data3[2] = { 0 };
	data3[0] = pProduct->expiryDate.day << 3 | pProduct->expiryDate.month >> 1;
	data3[1] = (pProduct->expiryDate.month & 0x1) << 7 | year << 4;


	if (fwrite(&data3, sizeof(BYTE), 2, fp) != 2)
		return 0;

	return 1;
}

int		loadProductFromFile(Product* pProduct, FILE* fp)
{
	if (fread(pProduct, sizeof(Product), 1, fp) != 1)
	{
		puts("Error reading product from file\n");
		return 0;
	}
	return 1;
}

char* concatenateValues(const char* prefix, int num1, int num2, int num3, int num4, int num5) {
	char* result = (char*)malloc(strlen(prefix) + 20);
	if (!result) return NULL;
	sprintf(result, "%s%d%d%d%d%d", prefix, num1, num2, num3, num4, num5);

	return result;
}

int	loadProductFromCompressedFile(Product* pProduct, FILE* fp)
{
	BYTE data[4];
	if (fread(&data, sizeof(BYTE), 4, fp) != 4)
		return 0;

	int barcode1 = (data[0] >> 4) & 0x0F;
	int barcode2 = (data[0]) & 0x0F;
	int barcode3 = (data[1] >> 4) & 0x0F;
	int barcode4 = (data[1]) & 0x0F;
	int barcode5 = (data[2] >> 4) & 0x0F;
	int type = (data[2] >> 2) & 0x03;
	eProductType  eType = (eProductType)(type);
	char tempB[BARCODE_LENGTH + 1];
	strcpy(tempB, getProductTypePrefix(eType));

	char* strBarcode = concatenateValues(tempB, barcode1, barcode2, barcode3, barcode4, barcode5);
	strcpy(pProduct->barcode, strBarcode);
	free(strBarcode);
	pProduct->type = eType;

	int len = ((data[2] & 0x03) << 2) | (data[3] >> 6 & 0x03);

	char* temp = (char*)calloc(len + 1, sizeof(char));
	if (!temp)
		return 0;

	if (fread(temp, sizeof(char), len, fp) != len)
	{
		free(pProduct->name);
		return 0;
	}
	strcpy(pProduct->name, temp);

	BYTE data2[3];
	if (fread(&data2, sizeof(BYTE), 3, fp) != 3)
		return 0;

	pProduct->count = data2[0];
	int fraction = (data2[1] >> 1);
	int whole = ((data2[1] & 0x1) << 8) | data2[2];

	pProduct->price = whole + (float)(fraction / 100.0);
	BYTE data3[2];
	if (fread(&data3, sizeof(BYTE), 2, fp) != 2)
		return 0;

	pProduct->expiryDate.day = data3[0] >> 3;
	pProduct->expiryDate.month = (data3[0] & 0x7) << 1 | data3[1] >> 7;
	int numOfYears = ((data3[1]) << 1) >> 5;
	pProduct->expiryDate.year = 2024 + numOfYears;
	return 1;
}

int	compareProductsByName(const void* prod1, const void* prod2)
{
	const Product* pProd1 = *(Product**)prod1;
	const Product* pProd2 = *(Product**)prod2;

	return strcmp(pProd1->name, pProd2->name);
}

int	compareProductsByCount(const void* prod1, const void* prod2)
{
	const Product* pProd1 = *(Product**)prod1;
	const Product* pProd2 = *(Product**)prod2;

	return pProd1->count - pProd2->count;
}

int	compareProductsByPrice(const void* prod1, const void* prod2)
{
	const Product* pProd1 = *(Product**)prod1;
	const Product* pProd2 = *(Product**)prod2;

	if (pProd1->price > pProd2->price)
		return 1;
	if (pProd1->price < pProd2->price)
		return -1;
	return 0;
}

void printProduct(const Product* pProduct)
{
	char* dateStr = getDateStr(&pProduct->expiryDate);
	printf("%-20s %-10s\t", pProduct->name, pProduct->barcode);
	printf("%-20s %5.2f %13d %7s %15s\n", typeStr[pProduct->type], pProduct->price, pProduct->count, " ", dateStr);
	free(dateStr);
}

void printProductPtr(void* v1)
{
	Product* pProduct = *(Product**)v1;
	printProduct(pProduct);
}

void generateBarcode(Product* pProd)
{
	char temp[BARCODE_LENGTH + 1];
	int barcodeNum;

	strcpy(temp, getProductTypePrefix(pProd->type));
	do {
		barcodeNum = MIN_BARCODE + rand() % (RAND_MAX - MIN_BARCODE + 1); //Minimum 5 digits
	} while (barcodeNum > MAX_BARCODE);

	sprintf(temp + strlen(temp), "%d", barcodeNum);

	strcpy(pProd->barcode, temp);
}

void getBarcodeCode(char* code)
{
	char temp[MAX_STR_LEN];
	char msg[MAX_STR_LEN];
	sprintf(msg, "Code should be of %d length exactly\n"
		"Must have %d type prefix letters followed by a %d digits number\n"
		"For example: FR20301",
		BARCODE_LENGTH, PREFIX_LENGTH, BARCODE_DIGITS_LENGTH);
	int ok = 1;
	int digCount = 0;
	do {
		ok = 1;
		digCount = 0;
		getsStrFixSize(temp, MAX_STR_LEN, msg);
		if (strlen(temp) != BARCODE_LENGTH)
		{
			puts("Invalid barcode length");
			ok = 0;
		}
		else
		{
			//check first PREFIX_LENGTH letters are upper case and valid type prefix
			char* typeSubStr = (char*)malloc(PREFIX_LENGTH + 1);
			if (!typeSubStr)
				return;
			strncpy(typeSubStr, temp, PREFIX_LENGTH);
			typeSubStr[PREFIX_LENGTH] = '\0';
			int prefixOk = 0;
			int i;

			for (i = 0; i < eNofProductType; i++)
			{
				if (strcmp(typeSubStr, typePrefix[i]) == 0)
				{
					prefixOk = 1;
					break; //found valid type prefix
				}
			}

			free(typeSubStr); //free the allocated memory

			if (!prefixOk)
			{
				puts("Invalid type prefix");
				ok = 0;
			}
			else
			{
				for (i = PREFIX_LENGTH; i < BARCODE_LENGTH; i++)
				{
					if (!isdigit(temp[i]))
					{
						puts("Only digits after type prefix\n");
						puts(msg);
						ok = 0;
						break;
					}
					digCount++;
				}

				if (digCount != BARCODE_DIGITS_LENGTH)
				{
					puts("Incorrect number of digits");
					ok = 0;
				}
			}
		}
	} while (!ok);

	strcpy(code, temp);
}

eProductType getProductType()
{
	int option;

	printf("\n");
	do {
		printf("Please enter one of the following types\n");
		for (int i = 0; i < eNofProductType; i++)
			printf("%d for %s\n", i, typeStr[i]);
		scanf("%d", &option);
	} while (option < 0 || option >= eNofProductType);

	getchar();

	return (eProductType)option;
}

const char* getProductTypeStr(eProductType type)
{
	if (type < 0 || type >= eNofProductType)
		return NULL;
	return typeStr[type];
}

const char* getProductTypePrefix(eProductType type)
{
	if (type < 0 || type >= eNofProductType)
		return NULL;
	return typePrefix[type];
}

int isProduct(const Product* pProduct, const char* barcode)
{
	if (strcmp(pProduct->barcode, barcode) == 0)
		return 1;
	return 0;
}

void updateProductCount(Product* pProduct)
{
	int count;

	do {
		printf("How many items to add to stock? ");
		scanf("%d", &count);
	} while (count < 1);

	pProduct->count += count;
}


void	freeProduct(Product* pProduct)
{
	//nothing to free!!!!
}