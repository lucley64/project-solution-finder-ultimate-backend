from currency_converter import CurrencyConverter
from json import dumps
from math import isnan
from pandas import read_csv
from sys import argv, exit
import os.path
import semantic_search

c = CurrencyConverter()

# Conversions not available in CurrencyConverter
french_franc_to_euro = 0.152449017
algerian_dinar_to_euro = 0.0068

# ref_currency is the short code for the currency used as reference (EUR, USD, etc.)
def balance_sheet(arr_sol_nb, df_gain_case_studies, df_cost_case_studies, ref_currency, df_currencies):
    """
    Returns the balance sheet for each solution number in arr_sol_nb

    Parameters
    ----------
    arr_sol_nb : array
        An array containing the solution numbers
    df_gain_case_studies : pandas.DataFrame
        A pandas Dataframe corresponding to the table tblgainrex of the dataset
    df_cost_case_studies : pandas.DataFrame
        A pandas Dataframe corresponding to the table tblcoutrex of the dataset
    ref_currency : str
        The currency code of the currency to use as reference (ex : "EUR", "USD", etc.)
    df_currencies : pandas.DataFrame
        A pandas Dataframe corresponding to the table tblmonnaie of the dataset
    
    Returns
    -------
    json
        A json
        "data_sol" : array of json with the following keys

        "nb_sol": the solution number
        "financial_median_cost" : the median cost associated with the solution
        "financial_max_cost" : the maximum cost associated with the solution
        "financial_min_cost" : the minimum cost associated with the solution
        "financial_median_gain" : the median gain associated with the solution
        "financial_max_gain" : the maximum gain associated with the solution
        "financial_min_gain" : the minimum gain associated with the solution

        The value associated with each key with the exception of the key "nb_sol" can be null
    """
    results = {"data_sol" : []}
    arr_eco_cost_per_sol = []
    arr_eco_gain_per_sol = []
    arr_energy_per_sol = []
    for i in range(0, len(arr_sol_nb)):
        nb_sol = arr_sol_nb[i]
        res_cost = eco_cost_bal_sheet_sol(nb_sol, df_cost_case_studies, ref_currency, df_currencies)
        arr_eco_cost_per_sol.append(res_cost)
        res_gain = eco_gain_bal_sheet_sol(nb_sol, df_gain_case_studies, ref_currency, df_currencies)
        arr_eco_gain_per_sol.append(res_gain)
    for i in range(0, len(arr_sol_nb)):
        data_sol = {
            "nb_sol": int(arr_sol_nb[i]),
            "financial_median_cost" : arr_eco_cost_per_sol[i]["median_cost"],
            "financial_max_cost" : arr_eco_cost_per_sol[i]["max_cost"],
            "financial_min_cost" : arr_eco_cost_per_sol[i]["min_cost"],
            "financial_median_gain" : arr_eco_gain_per_sol[i]["median_gain"],
            "financial_max_gain" : arr_eco_gain_per_sol[i]["max_gain"],
            "financial_min_gain" : arr_eco_gain_per_sol[i]["min_gain"]
        }
        results["data_sol"].append(data_sol)
    return(dumps(results))

def eco_cost_bal_sheet_sol(nb_sol, df_cost_case_studies, ref_currency, df_currencies):
    """
    Calculates the median cost of a solution (using its number) based on its case studies

    Parameters
    ----------
    nb_sol : int
        The solution number
    df_cost_case_studies : pandas.DataFrame
        A pandas Dataframe corresponding to the table tblcoutrex of the dataset
    ref_currency : str
        The currency code of the currency to use as reference (ex : "EUR", "USD", etc.)
    df_currencies : pandas.DataFrame
        A pandas Dataframe corresponding to the table tblmonnaie of the dataset
    
    Returns
    -------
    dict :
        "median_cost": the median cost
        "max_cost": the maximum cost
        "min_cost": the minimum cost
        The associated value for each key is a float rounded to the second decimal 
        or None if the calculation is not possible
    """
    arr_costs = []
    code_ref_currency = df_currencies.loc[df_currencies.shortmonnaie == ref_currency]
    # If the currency is not in the database, use euro by default
    if (code_ref_currency.empty):
        code_ref_currency = df_currencies.loc[df_currencies.shortmonnaie == "EUR"].values[0]
    else:
        code_ref_currency = code_ref_currency.nummonnaie.values[0]
    max_cost = -1
    min_cost = -1
    df_costs_one_sol = df_cost_case_studies.loc[df_cost_case_studies.codesolution == nb_sol]
    for j in range(0, len(df_costs_one_sol.index)):
        is_ignored = False
        cost_currency = df_currencies.loc[df_currencies.nummonnaie == df_costs_one_sol.codemonnaiecoutrex.values[j]]
        if (df_costs_one_sol.reelcoutrex.values[j] != None and not(isnan(df_costs_one_sol.reelcoutrex.values[j]))):
            cost = df_costs_one_sol.reelcoutrex.values[j]
        # If real cost unavailable, use min and max cost instead if available
        elif (df_costs_one_sol.minicoutrex.values[j] != None and df_costs_one_sol.maxicoutrex.values[j] != None
              and not(isnan(df_costs_one_sol.minicoutrex.values[j])) and not(isnan(df_costs_one_sol.maxicoutrex.values[j]))):
            cost = (df_costs_one_sol.minicoutrex.values[j] + df_costs_one_sol.maxicoutrex.values[j]) / 2
        else:
            is_ignored = True
            cost = 0
        if (not(is_ignored) and cost_currency.nummonnaie.values[0] != code_ref_currency):
                if (not(cost_currency.shortmonnaie.values[0] != cost_currency.shortmonnaie.values[0])):
                    if (cost_currency.shortmonnaie.values[0] not in c.currencies):
                        res = handle_unknown_currencies(cost_currency.shortmonnaie.values[0], cost)
                        if (res["total"] != None):
                            ref_currency_cost = c.convert(res["total"], res["currency_code"], ref_currency)
                        else:
                            ref_currency_cost = 0
                    else:
                        ref_currency_cost = c.convert(cost, cost_currency.shortmonnaie.values[0], ref_currency)
                # No currency associated, give up on value
                else:
                    ref_currency_cost = 0
        else:
            ref_currency_cost = cost
        if (ref_currency_cost != 0):
            arr_costs.append(ref_currency_cost)
            if (max_cost < ref_currency_cost or max_cost == -1):
                max_cost = round(ref_currency_cost, 2)
            if (min_cost > ref_currency_cost or min_cost == -1):
                min_cost = round(ref_currency_cost, 2)
    count = len(arr_costs)
    if (count > 0):
        # Calculation for the median
        # Sort in ascending order
        arr_costs.sort()
        # Case even number of values
        if (count % 2 == 0):
            index = int((count / 2)) - 1
            median_cost = round((arr_costs[index] + arr_costs[index + 1]) / 2, 2)
        # Case odd number of values
        else:
            index = int((count + 1) / 2) - 1
            median_cost = round(arr_costs[index], 2)
    else:
        median_cost = None
        max_cost = None
        min_cost = None
    res = {
        "median_cost": median_cost,
        "max_cost": max_cost,
        "min_cost": min_cost
    }
    return(res)

def eco_gain_bal_sheet_sol(nb_sol, df_gain_case_studies, ref_currency, df_currencies):
    """
    Calculates the median gain of a solution (using its number) based on its case studies

    Parameters
    ----------
    nb_sol : int
        The solution number
    df_gain_case_studies : pandas.DataFrame
        A pandas Dataframe corresponding to the table tblgainrex of the dataset
    ref_currency : str
        The currency code of the currency to use as reference (ex : "EUR", "USD", etc.)
    df_currencies : pandas.DataFrame
        A pandas Dataframe corresponding to the table tblmonnaie of the dataset
    
    Returns
    -------
    dict :
        "median_gain": the median gain
        "max_gain": the maximum gain
        "min_gain": the minimum gain
        The associated value for each key is a float rounded to the second decimal 
        or None if the calculation is not possible
    """
    arr_gains = []
    code_ref_currency = df_currencies.loc[df_currencies.shortmonnaie == ref_currency]
    # If the currency is not in the database, use euro by default
    if (code_ref_currency.empty):
        code_ref_currency = df_currencies.loc[df_currencies.shortmonnaie == "EUR"].values[0]
    else:
        code_ref_currency = code_ref_currency.nummonnaie.values[0]
    max_gain = -1
    min_gain = -1
    df_gains_one_sol = df_gain_case_studies.loc[df_gain_case_studies.codesolution == nb_sol]
    for j in range(0, len(df_gains_one_sol)):
        is_ignored = False
        gain_currency = df_currencies.loc[df_currencies.nummonnaie == df_gains_one_sol.codemonnaiegainrex.values[j]]
        if (df_gains_one_sol.gainfinanciergainrex.values[j] != None and not(isnan(df_gains_one_sol.gainfinanciergainrex.values[j]))):
            gain = df_gains_one_sol.gainfinanciergainrex.values[j]
        else:
            is_ignored = True
            gain = 0
        if (not(is_ignored) and gain_currency.nummonnaie.values[0] != code_ref_currency):
            if (not(gain_currency.shortmonnaie.values[0] != gain_currency.shortmonnaie.values[0])):
                if (gain_currency.shortmonnaie.values[0] not in c.currencies):
                    res = handle_unknown_currencies(gain_currency.shortmonnaie.values[0], gain)
                    if (res["total"] != None):
                        ref_currency_gain = c.convert(res["total"], res["currency_code"], ref_currency)
                    else:
                        ref_currency_gain = 0
                else:
                    ref_currency_gain = c.convert(gain, gain_currency.shortmonnaie.values[0], ref_currency)
            # No currency associated, give up on value
            else:
                ref_currency_gain = 0
        else:
            ref_currency_gain = gain
        if (ref_currency_gain != 0):
            arr_gains.append(ref_currency_gain)
            if (max_gain < ref_currency_gain or max_gain == -1):
                max_gain = round(ref_currency_gain, 2)
            if (min_gain > ref_currency_gain or min_gain == -1):
                min_gain = round(ref_currency_gain, 2)
    count = len(arr_gains)
    if (count > 0):
        # Calculation for the median
        # Sort in ascending order
        arr_gains.sort()
        # Case even number of values
        if (count % 2 == 0):
            index = int(count / 2) - 1
            median_gain = round((arr_gains[index] + arr_gains[index + 1]) / 2, 2)
        # Case odd number of values
        else:
            index = int((count + 1) / 2) - 1
            median_gain = round(arr_gains[index], 2)
    else:
        median_gain = None
        max_gain = None
        min_gain = None
    res = {
        "median_gain": median_gain,
        "max_gain": max_gain,
        "min_gain": min_gain
    }
    return(res)

def handle_unknown_currencies(str_currency_code, value):
    """
    Handles cases of unknown currencies within CurrencyConverter library by using rates to euros
    If the conversion to euros is possible then it is possible afterwards to convert from euros to a target currency known to CurrencyConverter

    Parameters
    ----------
    str_currency_code : str
        The currency code of the currency of value (ex : "EUR", "USD", etc.)
    value : float
        The value associated with the currency of currency code str_currency_code

    Returns
    -------
    dict
        Keys are "total" for the new value and "currency_code" for the new currency code
        The value associated with each key can be None if no conversion is possible
    """
    match str_currency_code:
        case "FNF":
            total = french_franc_to_euro * value
            currency_code = "EUR"
        case "LACS":
            total = 100000 * value
            currency_code = "IDR"
        case "DZD":
            total = algerian_dinar_to_euro * value
            currency_code = "DZD"
        # Unknown currency code
        case _:
            total = None
            currency_code = None

    return({"total": total, "currency_code": currency_code})

def energy_balance_sheet(nb_sol, df_case_studies, df_gain_case_studies, ref_currency, df_currencies):
    print()

def main():
    args = argv[1:]
    nb_args = len(args)
    match nb_args:
        case 1:
            query = args[0]
            ref_currency = "EUR"
        case 2:
            query = args[0]
            ref_currency = args[1]
        case _:
            exit("Missing at least one argument")
    res = semantic_search.semantic_search(query)
    gains_dataset = "./model/tblgainrex.csv"
    costs_dataset = "./model/tblcoutrex.csv"
    currencies_dataset = "./model/tblmonnaie.csv"
    df_gain_case_studies = read_csv(gains_dataset, sep = ',', engine = 'python', quotechar = '"')
    df_cost_case_studies = read_csv(costs_dataset, sep = ',', engine = 'python', quotechar = '"')
    df_currencies = read_csv(currencies_dataset, sep = ',', engine = 'python', quotechar = '"')
    results = balance_sheet(res, df_gain_case_studies, df_cost_case_studies, ref_currency, df_currencies)
    print(results)
    return(0)

path_embeddings = "./model/corpus_embeddings"
path_pickle = "./model/arr_nb_sol.pkl"

is_corpus_embeddings_to_update = False

if (not(os.path.isfile(path_embeddings)) or not(os.path.isfile(path_pickle))):
    is_corpus_embeddings_to_update = True

if (is_corpus_embeddings_to_update):
    dataset_path = "./model/textSolModel.csv"
    all_df = read_csv(dataset_path, sep = ',', engine = 'python', quotechar = '"')

    # Takes only text corresponding to the name of the solution and its definition (as written in map_params.json)
    # all_df = all_df.loc[(all_df.indexdictionnaire == "nomsolution") | (all_df.indexdictionnaire == "principesolution")]

    # The french text is the most complete as of now
    df = semantic_search.get_df_one_lang_one_sol_per_row(semantic_search.Language.FRENCH.value, all_df)

    df.traductiondictionnaire = df.traductiondictionnaire.apply(semantic_search.preprocess)

    # Randomize the rows associated at each index
    df = df.sample(frac = 1).reset_index(drop = True)

    semantic_search.encode_text_sols(df)

if __name__ == "__main__":
    main()