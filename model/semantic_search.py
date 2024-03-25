from re import sub
from enum import Enum
from pandas import concat, DataFrame
from sentence_transformers import SentenceTransformer, util
import torch
import pickle

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
    parsed_text = sub(space_pattern, ' ', parsed_text)
    parsed_text = sub(not_applicable_pattern, ' ', parsed_text)
    parsed_text = sub(tag_pattern, ' ', parsed_text)
    parsed_text = parsed_text.replace("\t", "")
    parsed_text = parsed_text.strip()
    return(parsed_text)

def encode_text_sols(df):
    device = torch.device('cuda' if torch.cuda.is_available() else 'cpu')
    corpus_embeddings = []
    arr_nb_sol = []
    str_sols_text = df.traductiondictionnaire.values
    for i in range(0, len(str_sols_text)):
        corpus_embeddings.append(str_sols_text[i])
        arr_nb_sol.append(df.loc[df.index == i].codeappelobjet.values[0])
    # paraphrase-multilingual-MiniLM-L12-v2
    embedder = SentenceTransformer("paraphrase-multilingual-mpnet-base-v2")
    corpus_embeddings = embedder.encode(corpus_embeddings, convert_to_tensor = True)
    # Store embeddings on disc
    with open("./embeddings.pkl", "wb") as fOut:
        pickle.dump({"arr_nb_sol": arr_nb_sol, "corpus_embeddings": corpus_embeddings}, fOut, protocol=pickle.HIGHEST_PROTOCOL)
    return(corpus_embeddings)

def semantic_search(query):
    device = torch.device('cuda' if torch.cuda.is_available() else 'cpu')
    # Load embeddings from disc
    with open("./embeddings.pkl", "rb") as fIn:
        stored_data = pickle.load(fIn)
        corpus_embeddings = stored_data["corpus_embeddings"]
        arr_nb_sol = stored_data["arr_nb_sol"]
    # paraphrase-multilingual-MiniLM-L12-v2
    embedder = SentenceTransformer("paraphrase-multilingual-mpnet-base-v2")
    query_embedding = embedder.encode(query, convert_to_tensor = True)
    results = util.semantic_search(query_embedding, corpus_embeddings, top_k = 5)
    list_nb_sol = []
    # corpus_id corresponds to the index of arr_nb_sol
    for i in range(0, len(results[0])):
        list_nb_sol.append(arr_nb_sol[results[0][i]['corpus_id']])
    return(list_nb_sol)
