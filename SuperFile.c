#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "General.h"
#include "FileHelper.h"
#include "SuperFile.h"
#include "Product.h"
#include "Macros.h"
typedef unsigned char BYTE;


int	saveSuperMarketToFile(const SuperMarket* pMarket, const char* fileName,
	const char* customersFileName)
{
	FILE* fp;
	fp = fopen(fileName, "wb");


	CHECK_MSG_RETURN_0(fp, "Error open supermarket file to write\n")

	
	if (!writeStringToFile(pMarket->name, fp, "Error write supermarket name\n"))
	{
	
		CLOSE_RETURN_0(fp);
	}

	

	if (!writeIntToFile(pMarket->productCount, fp, "Error write product count\n"))
	{
		CLOSE_RETURN_0(fp);
	}



	for (int i = 0; i < pMarket->productCount; i++)
	{
		if (!saveProductToFile(pMarket->productArr[i], fp))
		{
			CLOSE_RETURN_0(fp);
		}
	}

	fclose(fp);

	saveCustomersToTextFile(pMarket->customerArr, pMarket->customerCount, customersFileName);

	return 1;
}



int	saveSuperMarketToCompressedFile(const SuperMarket* pMarket, const char* fileName,
	const char* customersFileName)
{
	FILE* fp;
	fp = fopen(fileName, "wb");
	if (!fp) {
		printf("Error open supermarket file to write\n");
		return 0;
	}

	BYTE data[2] = { 0 };

	int len = (int)strlen(pMarket->name);

	data[0] = pMarket->productCount >>2;
	data[1] = ((pMarket->productCount) & 0x3)<<6;
	data[1] = len | data[1];

	if (fwrite(&data, sizeof(BYTE), 2, fp) != 2)
		return 0;

	if (fwrite(pMarket->name, sizeof(char), len, fp) != len)
		return 0;

	for (int i = 0; i < pMarket->productCount; i++)
	{
		if (!saveProductToCompressedFile(pMarket->productArr[i], fp))
		{
			CLOSE_RETURN_0(fp);
		}
	}

	fclose(fp);

	saveCustomersToTextFile(pMarket->customerArr, pMarket->customerCount, customersFileName);

	return 1;
}

int	loadSuperMarketFromFile(SuperMarket* pMarket, const char* fileName,
	const char* customersFileName)
{
	FILE* fp;
	fp = fopen(fileName, "rb");
	CHECK_MSG_RETURN_0(fp, "Error open company file\n");


	pMarket->name = readStringFromFile(fp, "Error reading supermarket name\n");
	if (!pMarket->name)
	{
		CLOSE_RETURN_0(fp);
	}

	int count;


	int val = readIntFromFile(&count, fp, "Error reading product count\n");
	FREE_CLOSE_FILE_RETURN_0(val,pMarket->name,fp);

	pMarket->productArr = (Product**)malloc(count * sizeof(Product*));
	FREE_CLOSE_FILE_RETURN_0(pMarket->productArr, pMarket->name,fp);

	pMarket->productCount = count;

	for (int i = 0; i < count; i++)
	{
		pMarket->productArr[i] = (Product*)malloc(sizeof(Product));

		FREE_CLOSE_FILE_RETURN_0(pMarket->productArr[i], pMarket->name,fp);

		if (!loadProductFromFile(pMarket->productArr[i], fp))
		{
			free(pMarket->productArr[i]);
			free(pMarket->name);
			fclose(fp);
			return 0;
		}
	}


	fclose(fp);

	pMarket->customerArr = loadCustomersFromTextFile(customersFileName, &pMarket->customerCount);
	if (!pMarket->customerArr)
		return 0;

	return	1;

}
int	loadSuperMarketFromCompressedFile(SuperMarket* pMarket, const char* fileName,const char* customersFileName)
{
	FILE* fp;
	fp = fopen(fileName, "rb");
	if (!fp)
	{
		printf("Error open company file\n");
		return 0;
	}

	BYTE data[2];
	if (fread(&data, sizeof(BYTE), 2, fp) != 2)
		return 0;

	int productCount = (data[0])<<2 | (data[1] >> 6 & 0x3);
	int nameLen = (data[1]& 0x3F);

	pMarket->name = (char*)calloc(nameLen + 1, sizeof(char));
	if (!pMarket->name)
		return 0;

	if (fread(pMarket->name, sizeof(char), nameLen, fp) != nameLen)
	{
		free(pMarket->name);
		return 0;
	}

	pMarket->productArr = (Product**)malloc(productCount * sizeof(Product*));
	if (!pMarket->productArr)
	{
		free(pMarket->name);
		fclose(fp);
		return 0;
	}

	pMarket->productCount = productCount;

	for (int i = 0; i < productCount; i++)
	{
		pMarket->productArr[i] = (Product*)malloc(sizeof(Product));
		if (!pMarket->productArr[i])
		{
			free(pMarket->name);
			fclose(fp);
			return 0;
		}

		if (!loadProductFromCompressedFile(pMarket->productArr[i], fp))
		{
			free(pMarket->productArr[i]);
			free(pMarket->name);
			fclose(fp);
			return 0;
		}
	}


	fclose(fp);

	pMarket->customerArr = loadCustomersFromTextFile(customersFileName, &pMarket->customerCount);
	if (!pMarket->customerArr)
		return 0;

	return	1;

}

int	saveCustomersToTextFile(const Customer* customerArr, int customerCount, const char* customersFileName)
{
	FILE* fp;

	fp = fopen(customersFileName, "w");
	if (!fp) {
		printf("Error opening customers file to write\n");
		return 0;
	}

	fprintf(fp, "%d\n", customerCount);
	for (int i = 0; i < customerCount; i++)
		customerArr[i].vTable.saveToFile(&customerArr[i], fp);

	fclose(fp);
	return 1;
}

Customer* loadCustomersFromTextFile(const char* customersFileName, int* pCount)
{
	FILE* fp;

	fp = fopen(customersFileName, "r");
	if (!fp) {
		printf("Error open customers file to write\n");
		return NULL;
	}

	Customer* customerArr = NULL;
	int customerCount;

	fscanf(fp, "%d\n", &customerCount);

	if (customerCount > 0)
	{
		customerArr = (Customer*)calloc(customerCount, sizeof(Customer)); //cart will be NULL!!!
		if (!customerArr)
		{
			fclose(fp);
			return NULL;
		}

		for (int i = 0; i < customerCount; i++)
		{
			if (!loadCustomerFromFile(&customerArr[i], fp))
			{
				freeCustomerCloseFile(customerArr, i, fp);
				return NULL;
			}
		}
	}

	fclose(fp);
	*pCount = customerCount;
	return customerArr;
}


void freeCustomerCloseFile(Customer* customerArr, int count, FILE* fp)
{
	for (int i = 0; i < count; i++)
	{
		free(customerArr[i].name);
		customerArr[i].name = NULL;
		if (customerArr[i].pDerivedObj)
		{
			free(customerArr[i].pDerivedObj);
			customerArr[i].pDerivedObj = NULL;
		}
	}
	free(customerArr);
	fclose(fp);
}