import re
from enum import Enum
from pandas import read_csv, concat, DataFrame
from sentence_transformers import SentenceTransformer, util
import torch

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

def semantic_search(df, query):
    device = torch.device('cuda' if torch.cuda.is_available() else 'cpu')
    print('Using device:', device)
    print()
    if device.type == 'cuda':
        print(torch.cuda.get_device_name(0))
        print('Memory Usage:')
        print('Allocated:', round(torch.cuda.memory_allocated(0)/1024**3,1), 'GB')
        print('Cached:   ', round(torch.cuda.memory_reserved(0)/1024**3,1), 'GB')
    corpus_embeddings = []
    # Give up msmarco-MiniLM-L-6-v3, probably not possible to find a usable multilingual msmarco model
    # embedder = SentenceTransformer("msmarco-MiniLM-L-6-v3")

    # Comparison for the same query
    # Important : Huge loss of accuracy for paraphrase-multilingual-MiniLM-L12-v2 with query "J'aimerais avoir une régulation optimisée de mon groupe froid"
    # paraphrase-multilingual-mpnet-base-v2 -> About 50s, highest score on a random run -> About 0.80, note : seems more consistent with the results
    # paraphrase-multilingual-MiniLM-L12-v2 -> About 18s, highest score on a random run -> About 0.735
    # embedder = SentenceTransformer("paraphrase-multilingual-mpnet-base-v2")
    embedder = SentenceTransformer("paraphrase-multilingual-mpnet-base-v2")
    query_embedding = embedder.encode(query, convert_to_tensor = True)
    str_sols_text = df.traductiondictionnaire.values
    for i in range(0, len(str_sols_text)):
        corpus_embeddings.append(str_sols_text[i])
    corpus_embeddings = embedder.encode(corpus_embeddings, convert_to_tensor = True)
    # Without normalize
    results = util.semantic_search(query_embedding, corpus_embeddings, top_k = 5)
    # corpus_id should correspond to the index of the row within df
    print("-------------------------------")
    # Check how to generate csv file with all sectors and then take the one needed
    for i in range(0, len(results[0])):
        print("corpus_id : " + str(results[0][i]['corpus_id']))
        print("score : " + str(results[0][i]['score']))
        print(df.loc[df.index == results[0][i]['corpus_id']])
        print("-------------------------------------")
    return(results)

dataset_path = "./model/textSolModel.csv"
all_df = read_csv(dataset_path, sep = ',', engine = 'python', quotechar = '"')

df = get_df_one_lang_one_sol_per_row(Language.FRENCH.value, all_df)

df.traductiondictionnaire = df.traductiondictionnaire.apply(preprocess)

# Randomize the rows associated at each index
df = df.sample(frac = 1).reset_index(drop = True)

# ex_query = "Comment faire pour réduire la consommation de mon compresseur d'air comprimé ?"
ex_query = "J'aimerais avoir une régulation optimisée de mon groupe froid"
# ex_query = "C'est quoi la haute pression flottante"
# ex_query = "Je voudrais dimensionner un panneau solaire."
# ex_query = "Quel gain pour un variateur de vitesse ?"
# ex_query = "Quelles sont les meilleures solutions pour l'agro-alimentaire ?"

semantic_search(df, ex_query)
