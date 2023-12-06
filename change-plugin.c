#include <stdio.h>
#include <regex.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

void writeToFile(const char *filename, const char *content) {
    // Open the file in write mode ("w")
    FILE *file = fopen(filename, "w");

    if (file == NULL) {
        fprintf(stdout, "Error opening file\n");
        return;
    }

    // Write the content to the file
    fputs(content, file);

    // Close the file
    fclose(file);
}

void removeFileExtension(char * filename){
    char * extension = strchr(filename,'.');
    *extension='\0';
}

/*  saveJMX("mytest.jmx", file_content) 
    creates a new file called mytest-temp.jmx and print it's name in stdout
*/
void saveJMX(char * jmxFilename, char * content){
    
    removeFileExtension(jmxFilename);
    char filenameTemp[1000];
    strcpy(filenameTemp,jmxFilename);
    strcat(filenameTemp,"-temp.jmx");
    writeToFile(filenameTemp,content);
    printf("\n%s\n",filenameTemp);
}

/*given an input and a regex match, returns a new (malloc'ed) char * containing the matching substring*/
char * getMatchingString(const char *input, regmatch_t *match) {
    
    if (match->rm_so != -1 && match->rm_eo != -1) {
        int len = match->rm_eo-match->rm_so;
        char * s = (char *) malloc(sizeof(char)*len+1);
        assert(s);
        int l=0;
        for (int i = match->rm_so; i < match->rm_eo; ++i,l++) {
            s[l]=input[i];
        }
        s[l]='\0';
        return s;
    } else {
        return NULL;
    }
}

void printMatchingString(const char *input, regmatch_t *match) {
    if (match->rm_so != -1 && match->rm_eo != -1) {
        for (int i = match->rm_so; i < match->rm_eo; ++i) {
            putchar(input[i]);
        }
        printf("\n");
    } else {
        printf("No match found.\n");
    }
}

FILE *openFile(const char *filename, const char *mode) {
    FILE *file = fopen(filename, mode);

    if (file == NULL) {
        fprintf(stdout, "Error opening file: %s\n",filename);
    }

    return file;
}

char *readFileToString(const char *filename) {
    FILE *file = openFile(filename, "r");
    if (file == NULL) {
        return NULL;
    }

    // Seek to the end of the file to determine its size
    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);

    // Allocate memory for the entire file content plus one extra character for the null terminator
    char *fileContent = (char *)malloc(fileSize + 1);
    
    if (fileContent == NULL) {
        perror("Error allocating memory");
        fclose(file);
        return NULL;
    }

    // Read the entire file into fileContent
    fread(fileContent, 1, fileSize, file);

    // Null-terminate the string
    fileContent[fileSize] = '\0';

    // Close the file
    fclose(file);

    return fileContent;
}

int countMatchesCSV(char *_String,int *_offset){
    // Variable to store initial regex()
    regex_t reegexCSV;

    // Variable for return type
    int value;
    regmatch_t match;
    // Creation of regEx

    if(regcomp(&reegexCSV,"<CSVDataSet.{0,1000}?<\\/CSVDataSet>",REG_EXTENDED)!=0)
        return 1;

    int offset=0,matches=0;
    while (regexec(&reegexCSV, _String + offset, 1, &match, 0) == 0) {
        matches++;
            
        // Print the matching string
        //printf("Match at offset %d:\n", offset + (*__pmatch).rm_so);
        //printMatchingString(_String + offset,__pmatch);

        // Move the offset to the end of the current match
        offset += match.rm_eo;

        //returning the offset to the main function so we know the size of the 'steps' when looking for multiple matches.
        if(matches==1) 
            *_offset=offset; 
        
        // Break the loop if there are no more matches
        if (match.rm_so == -1) {
            break;
        }

    }
    return matches;
}

void getVariables(char *input,char ** _filename,char ** _ignoreFirstLine,char ** _recycle, char ** _variables){

    regex_t reegexFilename,reegexIgnoreFirstLine,reegexRecycle,reegexVariables,reegexCSV;

    char *filename,*ignoreFirstLine,*recycle,*variables;
    // Variable for return type
    int value;
    regmatch_t match[5];
    // Creation of regEx
    
    assert(regcomp(&reegexFilename,"\"filename\">([a-zA-Z0-9\\.]{4,50})<",REG_EXTENDED)==0);
    assert(regcomp(&reegexIgnoreFirstLine,"\"ignoreFirstLine\">([a-z]+)<",REG_EXTENDED)==0);
    assert(regcomp(&reegexRecycle,"\"recycle\">([a-z]+)<",REG_EXTENDED)==0);
    assert(regcomp(&reegexVariables,"\"variableNames\">([a-zA-Z0-9!@#$¨&*,]+)<\\/stringProp>",REG_EXTENDED)==0);

    int offset=0,matches=0;
    if(regexec(&reegexFilename, input, 2, match, 0) == 0){
       // printf("%p\n",getMatchingString(input,&match[1]));
       *_filename = getMatchingString(input,&match[1]);
    }
    if(regexec(&reegexIgnoreFirstLine, input, 2, match, 0) == 0){
      //  printf("%s\n",getMatchingString(input,&match[1]));
        *_ignoreFirstLine = getMatchingString(input,&match[1]);
    }
    if(regexec(&reegexRecycle, input, 2, match, 0) == 0){
      //  printf("%s\n",getMatchingString(input,&match[1]));
        *_recycle = getMatchingString(input,&match[1]);
    }
    if(regexec(&reegexVariables, input, 2, match, 0) == 0){
      //  printf("%s\n",getMatchingString(input,&match[1]));
        *_variables = getMatchingString(input,&match[1]);
    }
    //free the regex structures when done
    regfree(&reegexVariables);
    regfree(&reegexRecycle);
    regfree(&reegexIgnoreFirstLine);
    regfree(&reegexFilename);
}

char *replaceSubstring(const char *input, const char *pattern, const char *replacement) {
    regex_t regex;
    regmatch_t match;
    int offset = 0;
    size_t resultSize = 0;
    char *result = NULL;
    size_t start=0,end=0,finalLen = 0;
    // Compile the regex
    int regexCompileResult = regcomp(&regex, pattern, REG_EXTENDED);
 

    //T H E 'S T R M A T C H' R E S T - input
    //M Y R E P L A C E M E N T S T R - replacement

    //T H E 'M Y R E P L A C E M E N T S T R' R E S T - result
    if (regexCompileResult == 0) {
        // Calculate the size of the result string
        if (regexec(&regex, input, 1, &match, 0) != 0) {
            return NULL;
        }

        start = match.rm_so;
        end = match.rm_eo;

        resultSize = end - start;
            
        if(strlen(replacement)> resultSize)
            result = (char *) malloc(sizeof(char) * strlen(input) + strlen(replacement) - resultSize+1);
        finalLen = strlen(input) + strlen(replacement) - resultSize+1;

        //printf("\n\tresultSize%ld\n",resultSize);
        
        if (result != NULL) {
            // Copy the input string with replacements
            size_t currentIndex = 0;
           
            if (regexec(&regex, input, 1, &match, 0) == 0) {
                // Copy characters before the match
                strncpy(result, input, match.rm_so);
                currentIndex += match.rm_so;

                // If there's a match, copy the replacement
                if (match.rm_so != -1) {
                    strcpy(result + currentIndex, replacement);
                    currentIndex += strlen(replacement);
                    // Copy the remaining characters after the last match
                    strcpy(result + currentIndex, input +match.rm_eo);
                }

            }
        }

        regfree(&regex);
    } else {
        // Handle regex compilation error
        char errorBuffer[100];
        regerror(regexCompileResult, &regex, errorBuffer, sizeof(errorBuffer));
        fprintf(stdout, "Error: Regex compilation error: %s\n", errorBuffer);
    }
   // printf("\n\tresult%s\n",result);
    return result;
}

int main (int argc, char** argv){
    if(argc <3){
        fprintf(stdout,"Error:falta um parâmetro: arquivo JMX\nExemplo: './change-plugin temp.txt POC.jmx'\n");
        return 1;
    }
    char pluginRedisExampleFileName [30] = "pluginRedis.example";
    char * jmxFileName = argv[2]; // get jmx filename from args
    char * tempFileName = argv[1]; // get temporary filename containing csv element
    char * jmxFileContent = readFileToString(jmxFileName);
    char * pluginRedisExampleContent = readFileToString(pluginRedisExampleFileName);
    char * tempFileContent = readFileToString(tempFileName);
    char * auxPluginRedisOutput = (char *) malloc (sizeof(char)*strlen(pluginRedisExampleContent)+500);
         
    if(tempFileContent==NULL){
        return 1;
    }
    if(jmxFileContent==NULL){
        return 1;
    }
    if(pluginRedisExampleContent==NULL){
        return 1;
    }

    int offset = 0;
    int count = countMatchesCSV(tempFileContent,&offset);
    if(count <1){
        fprintf(stdout, "Error: No matches found at file: '%s'\n",tempFileName);
        return 1;
    }
    
    int i =0;
    char *filename,*ignoreFirstLine,*recycle,*variables;
    
    for(i=0;i<count;i++){
        getVariables(tempFileContent+offset*i,&filename,&ignoreFirstLine,&recycle,&variables);
        removeFileExtension(filename);
        //printf("offset %d - %s,%s,%s,%s\n",offset,filename,ignoreFirstLine,recycle,variables);
        
        //write formated pluginRedisExampleContent to auxPluginRedisOutput containing 'recycle,filename and variables' values
        if(sprintf(auxPluginRedisOutput, pluginRedisExampleContent, recycle,filename,variables)<0){
            fprintf(stdout, "Error: Could not format plugin jmx output\n");
            return 1;
        }

        //printf("%s\n",auxPluginRedisOutput);

        //replace matching <CSVDataset> elements with 'pluginRedisExampleContent' at jmxFileContent
        jmxFileContent = replaceSubstring(jmxFileContent,"<CSVDataSet.{0,100}enabled=\"true\".{0,1000}?<\\/CSVDataSet>",auxPluginRedisOutput);

    }

    saveJMX(jmxFileName,jmxFileContent);
    return 0;
}


