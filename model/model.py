import re
from pandas import read_csv

def preprocess(string_text):
    space_pattern = '\s+'
    parsed_text = re.sub(space_pattern, ' ', string_text)
    parsed_text = parsed_text.replace("\t", "")
    parsed_text = parsed_text.strip()
    return(parsed_text)

dataset_path = "./model/textSolModel.csv"
all_df = read_csv(dataset_path)
print(all_df)
