import re
from enum import Enum
from pandas import read_csv, concat, DataFrame

class Language(Enum):
    FRENCH = 2
    ENGLISH = 3
    SPANISH = 4

def get_df_one_lang_one_sol_per_row(code_language, all_df):
    # Format of the all_df dataframe and returned dataframe
    df = DataFrame(data = {"codelangue" : [], "codeappelobjet" : [], "traductiondictionnaire" : []})
    one_language_df = all_df.loc[all_df.codelangue == code_language]
    one_language_df_length = len(one_language_df.index)
    if (one_language_df_length > 0):
        code_sol = one_language_df.codeappelobjet[0]
        row_start = 0
        row_end = 1
        while (row_end < one_language_df_length):
            if (one_language_df.codeappelobjet.values[row_end] == code_sol):
                row_end = row_end + 1
            else:
                str_text_one_sol = one_language_df.traductiondictionnaire.values[row_start]
                if (str_text_one_sol[len(str_text_one_sol) - 1] != '.'):
                    str_text_one_sol = str_text_one_sol + '. '
                for i in range(row_start + 1, row_end):
                    str_text = one_language_df.traductiondictionnaire.values[i]
                    if (str_text[len(str_text) - 1] != '.'):
                        str_text = str_text + '. '
                    str_text_one_sol = str_text_one_sol + str_text
                new_row = {"codelangue" : code_language, "codeappelobjet" : one_language_df.codeappelobjet.values[row_start], "traductiondictionnaire" : str_text_one_sol}
                df = concat([df, DataFrame([new_row])], ignore_index = True)
                row_start = row_end
                code_sol = one_language_df.codeappelobjet.values[row_start]
                row_end = row_end + 1
        str_text_one_sol = one_language_df.traductiondictionnaire.values[row_start]
        if (str_text_one_sol[len(str_text_one_sol) - 1] != '.'):
            str_text_one_sol = str_text_one_sol + '. '
        for i in range(row_start + 1, row_end):
            str_text = one_language_df.traductiondictionnaire.values[i]
            if (str_text[len(str_text) - 1] != '. '):
                str_text = str_text + '. '
            str_text_one_sol = str_text_one_sol + str_text
        new_row = {"codelangue" : code_language, "codeappelobjet" : one_language_df.codeappelobjet.values[row_start], "traductiondictionnaire" : str_text_one_sol}
        df = concat([df, DataFrame([new_row])], ignore_index = True)
    else:
        raise Exception("Entry dataframe parameter is empty")
    df = df.convert_dtypes()
    return(df)

def preprocess(string_text):
    parsed_text = str(string_text)
    space_pattern = "\\s+"
    not_applicable_pattern = "- N/A -"
    tag_pattern = "<[^>]*>"
    parsed_text = re.sub(space_pattern, ' ', parsed_text)
    parsed_text = re.sub(not_applicable_pattern, ' ', parsed_text)
    parsed_text = re.sub(tag_pattern, ' ', parsed_text)
    parsed_text = parsed_text.replace("\t", "")
    parsed_text = parsed_text.strip()
    return(parsed_text)

dataset_path = "./model/textSolModel.csv"
all_df = read_csv(dataset_path, sep = ',', engine = 'python', quotechar = '"')

all_df = get_df_one_lang_one_sol_per_row(Language.FRENCH.value, all_df)

all_df.traductiondictionnaire = all_df.traductiondictionnaire.apply(preprocess)

# Randomize the rows associated at each index
all_df = all_df.sample(frac = 1).reset_index(drop = True)

print(all_df)
