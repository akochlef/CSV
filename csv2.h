/* 
CSV 2.0 class definition
Anis Kochlef - 11/7/2019
Last update: 11/14/19
*/

#ifndef _GLIBCXX_IOSTREAM
#include<iostream>
#endif
#ifndef _GLIBCXX_FSTREAM
#include <fstream>
#endif
#ifndef _GLIBCXX_VECTOR
#include <vector>
#endif
#ifndef _GLIBCXX_STRING
#include <string>
#endif
#ifndef _GLIBCXX_SET
#include <set>
#endif

#ifndef __CSV2__
#define __CSV2__ 1

#define ON 1
#define OFF 0
#define MAX_FIELD_SIZE	1024

class CSV
{
	private:
	int R,C;
	int status;
	std::vector<std::vector<std::string>> DATA;
	std::string FILENAME;
	// int tokenize(std::string *line,std::vector<std::string> *token,char separator,char delimiter);
	int escape_delimiter(std::string *str);
	public:
	int tokenize(std::string *line,std::vector<std::string> *token,char separator,char delimiter);
	CSV(){DATA.clear();R=C=status=0;}
	~CSV(){DATA.clear();R=C=status=0;}
	int getNRows(){return R;}
	int getNColumns(){return C;}
	int readCSV(const char *filename,char separator,char delimiter);
	int writeCSV(const char *filename);
	void newCSV(int row,int column,const std::string defaultValue);
	// add column, drop column, add row and drop row
	int getField(int row,int column,std::string *field);
	int setField(int row,int column,const std::string field);
	void operator = (CSV const &obj) {DATA = obj.DATA; R = obj.R; C = obj.C;status=obj.status;}
	void inline dropRow(int row);
	void dropColumn(int column);
	int dropRowByFilter(int column,std::string value);
};

void CSV::dropRow(int row) 
{
	if((row>=0)&&(row<R))
	{
		DATA.erase(DATA.begin()+row);
		R--;
	}
}

void CSV::dropColumn(int column) 
{
	int i;
	if((column>=0)&&(column<C))
	{
		for(i=0;i<R;i++)
		{
			DATA[i].erase(DATA[i].begin()+column);
		}
		C--;
	}
}

int CSV::dropRowByFilter(int column,std::string value)
{
	std::string field;
	int r=0;
	int i=0;
	time_t t1,t2;
	t1=time(NULL);
	while(i<R)
	{
			if(DATA[i][column]==value)
			{
				dropRow(i);
				r++;
			} else i++;
	}
	t2=time(NULL);
	// std::cout << value << " dropped in " << (t2-t1) << " seconds.\n";
	// std::cout << "[new] dropped " << r << " rows.\n";
	return r;
}


int CSV::escape_delimiter(std::string *str)
{
	size_t index = 0;
	unsigned int count=0;
	while (true) 
	{
		 /* Locate the substring to replace. */
		 index = str->find(std::string("\""), index);
		 if (index == std::string::npos) break;

		 /* Make the replacement. */
		 //str->replace(index, dst.length(), dst);
		 str->insert(index,std::string("\"")); 
		 count++;
		 /* Advance index forward so the next iteration doesn't pick it up as well. */
		 index += 2;
	}
	return count;
}


int CSV::tokenize(std::string *line,std::vector<std::string> *token,char separator,char delimiter)
{
	char field[MAX_FIELD_SIZE];
	int delimiter_status=OFF;
	int escape_status=OFF;
	int i=0,j=0;
	field[0]='\0';
	std::string::iterator it,next_it;
	
	// Clean token container
	token->erase(token->begin(),token->end());
	
	for ( std::string::iterator it=line->begin(); it!=line->end(); ++it)
	{
		if(*it==delimiter)
		{
			if(delimiter_status==OFF)
			{
				delimiter_status = ON;
			}
			else
			{ 
				if(escape_status==ON)
				{
					field[i++]=delimiter;
					escape_status=OFF;
				}
				else
				{
					next_it = it+1;
					if((next_it!=line->end())&&(*next_it==delimiter))
						escape_status=ON;
					else
						delimiter_status=OFF;
				}
			}
		} else 
		{
			escape_status=OFF;
			if (*it==separator)
				{
					
					if(delimiter_status==OFF)
					{
						field[i]='\0';
						i=0;
						token->push_back(std::string(field));
						// std::cout << ".";
						//cout << ":" << string(field);
						j++;
					} else 
						{
							field[i++]=separator;
						}
				} else
					{
						field[i++]=*it;
					}
		}
	}
	
	field[i]='\0';
	token->push_back(std::string(field));
	// std::cout << ".\n";
	//cout << ":" << string(field) << "\n";
	j++;
	return j;
}

void CSV::newCSV(int row,int column,const std::string defaultValue)
{
	std::vector<std::string> defaultRow;
	int i;
	for (i=0;i<column;i++)
		defaultRow.push_back(defaultValue);
	for (i=0;i<row;i++)
		DATA.push_back(defaultRow);
	R=row;C=column;status=1;
}

int CSV::readCSV(const char *filename,char separator,char delimiter)
{
	int t;
	std::vector<std::string> token;
	std::string line;
	std::ifstream is(filename);
  	
	if(!status) // open file only when CSV uninitialized.
	{
		C=R=0;
		status=1;
		while (getline(is,line))
		{
			/* Fixed on 2020/05/21 : removes carriage return in the last field*/
			if (!line.empty() && line[line.size() - 1] == '\r')
				line.erase(line.size() - 1);
			if (!line.empty() && line[line.size() - 1] == '\n')
				line.erase(line.size() - 1);
			if (!line.empty() && line[line.size() - 1] == '\r')
				line.erase(line.size() - 1);
			/*End of Fix*/
			t=tokenize(&line,&token,separator,delimiter);
			DATA.push_back(token);
			if(t>C) C=t;
			R++;
			token.clear();
		}
	}
  	is.close();
	return R;
}

int CSV::writeCSV(const char *filename)
{
	int i,j,r=0;
	
	std::string field;
	if(status) // creates the file only when the CSV has been initialized.
	{
		std::ofstream os(filename);
		if(os)
		{
			r = 1;
			for (i=0;i<R;i++)
				for (j=0;j<C;j++)
				{
					field = DATA[i][j];
					escape_delimiter(&field);
					os << "\"" << field << "\"";
					//if(j<(C-1)) os << ","; else if(i<(R-1)) os << "\n";
					if(j<(C-1)) os << ","; else os << "\n";
				}
			os.close();
		}
	}
	return r;
}


int CSV::getField(int row,int column,std::string *field)
{
	int i=0;
	
	*field=std::string(""); // fix on 2020-10-07
	
	if((row>=0)&&(row<R)&&(column>=0)&&(column<C))
	{
		if(column<DATA[row].size()) // fix on 2020-10-07
		{
			i=1;
			*field = DATA[row][column];
		}
	}
	return i;
}

int CSV::setField(int row,int column,const std::string field)
{
	int i=0;
	if((row>=0)&&(row<R)&&(column>=0)&&(column<C))
	{
		i=1;
		DATA[row][column]=field;
	}
	return i;
}

int exportStringSet2CSV(const char *filename,std::set<std::string> s)
{
	CSV out;
	unsigned int i=0;
	std::set<std::string>::iterator it;
	out.newCSV(s.size(),1,std::string(""));
	for(it=s.begin();it!=s.end();++it)
	{
		out.setField(i++,0,*it);
	}
	out.writeCSV(filename);
	return i;
}

int exportStringVector2CSV(const char *filename,std::vector<std::string> s)
{
	CSV out;
	unsigned int i=0;
	std::vector<std::string>::iterator it;
	out.newCSV(s.size(),1,std::string(""));
	for(it=s.begin();it!=s.end();++it)
	{
		out.setField(i++,0,*it);
	}
	out.writeCSV(filename);
	return i;
}


#endif
