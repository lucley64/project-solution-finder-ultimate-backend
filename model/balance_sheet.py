from pandas import read_csv
from currency_converter import CurrencyConverter
from json import dumps
from math import isnan
import semantic_search

c = CurrencyConverter()

# Conversions not available in CurrencyConverter
french_franc_to_euro = 0,152449017
algerian_dinar_to_euro = 0.0068

# ref_currency is the short code for the currency used as reference (EUR, USD, etc.)
def balance_sheet(arr_sol_nb, df_case_studies, df_gain_case_studies, df_cost_case_studies, ref_currency, df_currencies):
    results = {"data_sol" : []}
    arr_eco_cost_per_sol = []
    arr_eco_gain_per_sol = []
    arr_energy_per_sol = []
    for i in range(0, len(arr_sol_nb)):
        nb_sol = arr_sol_nb[i]
        avg_cost = eco_cost_bal_sheet_sol(nb_sol, df_cost_case_studies, ref_currency, df_currencies)
        arr_eco_cost_per_sol.append(avg_cost)
        avg_gain = eco_gain_bal_sheet_sol(nb_sol, df_gain_case_studies, ref_currency, df_currencies)
        arr_eco_gain_per_sol.append(avg_gain)
    for i in range(0, len(arr_sol_nb)):
        data_sol = {
            "nb_sol": int(arr_sol_nb[i]),
            "financial_cost" : arr_eco_cost_per_sol[i],
            "financial_gain" : arr_eco_gain_per_sol[i]
        }
        results["data_sol"].append(data_sol)
    print(results)
    return(dumps(results))

def eco_cost_bal_sheet_sol(nb_sol, df_cost_case_studies, ref_currency, df_currencies):
    arr_costs = []
    code_ref_currency = df_currencies.loc[df_currencies.shortmonnaie == ref_currency]
    # If the currency is not in the database, use euro by default
    if (code_ref_currency.empty):
        code_ref_currency = df_currencies.loc[df_currencies.shortmonnaie == "EUR"].values[0]
    else:
        code_ref_currency = code_ref_currency.nummonnaie.values[0]
    count = 0
    df_costs_one_sol = df_cost_case_studies.loc[df_cost_case_studies.codesolution == nb_sol]
    for j in range(0, len(df_costs_one_sol.index)):
        is_ignored = False
        cost_currency = df_currencies.loc[df_currencies.nummonnaie == df_costs_one_sol.codemonnaiecoutrex.values[j]]
        if (df_costs_one_sol.reelcoutrex.values[j] != None and not(isnan(df_costs_one_sol.reelcoutrex.values[j]))):
            cost = df_costs_one_sol.reelcoutrex.values[j]
            count = count + 1
        # If real cost unavailable, use min and max cost instead if available
        elif (df_costs_one_sol.minicoutrex.values[j] != None and df_costs_one_sol.maxicoutrex.values[j] != None
              and not(isnan(df_costs_one_sol.minicoutrex.values[j])) and not(isnan(df_costs_one_sol.maxicoutrex.values[j]))):
            cost = (df_costs_one_sol.minicoutrex.values[j] + df_costs_one_sol.maxicoutrex.values[j]) / 2
            count = count + 1
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
                            count = count - 1
                            ref_currency_cost = 0
                    else:
                        ref_currency_cost = c.convert(cost, cost_currency.shortmonnaie.values[0], ref_currency)
                # No currency associated, give up on value
                else:
                    count = count - 1
                    ref_currency_cost = 0
        else:
            ref_currency_cost = cost
        if (ref_currency_cost != 0):
            arr_costs.append(ref_currency_cost)
    if (count > 0):
        # Calculation for the median
        # Sort in ascending order
        arr_costs.sort()
        # Case even number of values
        if (count % 2 == 0):
            index = int((count / 2)) - 1
            median_cost = float((arr_costs[index] + arr_costs[index + 1]) / 2)
        # Case odd number of values
        else:
            index = int((count + 1) / 2) - 1
            median_cost = float(arr_costs[index])
    else:
        median_cost = None
    return(median_cost)

def eco_gain_bal_sheet_sol(nb_sol, df_gain_case_studies, ref_currency, df_currencies):
    arr_gains = []
    code_ref_currency = df_currencies.loc[df_currencies.shortmonnaie == ref_currency]
    # If the currency is not in the database, use euro by default
    if (code_ref_currency.empty):
        code_ref_currency = df_currencies.loc[df_currencies.shortmonnaie == "EUR"].values[0]
    else:
        code_ref_currency = code_ref_currency.nummonnaie.values[0]
    count = 0
    df_gains_one_sol = df_gain_case_studies.loc[df_gain_case_studies.codesolution == nb_sol]
    for j in range(0, len(df_gains_one_sol)):
        is_ignored = False
        gain_currency = df_currencies.loc[df_currencies.nummonnaie == df_gains_one_sol.codemonnaiegainrex.values[j]]
        if (df_gains_one_sol.gainfinanciergainrex.values[j] != None and not(isnan(df_gains_one_sol.gainfinanciergainrex.values[j]))):
            gain = df_gains_one_sol.gainfinanciergainrex.values[j]
            count = count + 1
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
                        count = count - 1
                        ref_currency_gain = 0
                else:
                    ref_currency_gain = c.convert(gain, gain_currency.shortmonnaie.values[0], ref_currency)
            # No currency associated, give up on value
            else:
                count = count - 1
                ref_currency_gain = 0
        else:
            ref_currency_gain = gain
        if (ref_currency_gain != 0):
            arr_gains.append(ref_currency_gain)
    if (count > 0):
        # Calculation for the median
        # Sort in ascending order
        arr_gains.sort()
        # Case even number of values
        if (count % 2 == 0):
            index = int(count / 2) - 1
            median_gain = float((arr_gains[index] + arr_gains[index + 1]) / 2)
        # Case odd number of values
        else:
            index = int((count + 1) / 2) - 1
            median_gain = float(arr_gains[index])
    else:
        median_gain = None
    return(median_gain)

def handle_unknown_currencies(str_currency_code, value):
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


case_studies_dataset = "./model/tblrex.csv"
gains_dataset = "./model/tblgainrex.csv"
costs_dataset = "./model/tblcoutrex.csv"
currencies_dataset = "./model/tblmonnaie.csv"
df_case_studies = read_csv(case_studies_dataset, sep = ',', engine = 'python', quotechar = '"')
df_gain_case_studies = read_csv(gains_dataset, sep = ',', engine = 'python', quotechar = '"')
df_cost_case_studies = read_csv(costs_dataset, sep = ',', engine = 'python', quotechar = '"')
df_currencies = read_csv(currencies_dataset, sep = ',', engine = 'python', quotechar = '"')

dataset_path = "./model/textSolModel.csv"
all_df = read_csv(dataset_path, sep = ',', engine = 'python', quotechar = '"')

df = semantic_search.get_df_one_lang_one_sol_per_row(semantic_search.Language.FRENCH.value, all_df)

df.traductiondictionnaire = df.traductiondictionnaire.apply(semantic_search.preprocess)

# Randomize the rows associated at each index
df = df.sample(frac = 1).reset_index(drop = True)

ex_query = "Comment faire pour réduire la consommation de mon compresseur d'air comprimé ?"
# ex_query = "J'aimerais avoir une régulation optimisée de mon groupe froid"
# ex_query = "C'est quoi la haute pression flottante"
# ex_query = "Je voudrais dimensionner un panneau solaire."
# ex_query = "Quel gain pour un variateur de vitesse ?"
# ex_query = "Quelles sont les meilleures solutions pour l'agro-alimentaire ?"

is_corpus_embeddings_to_update = False

if (is_corpus_embeddings_to_update):
    semantic_search.encode_text_sols(df)

res = semantic_search.semantic_search(ex_query)

balance_sheet(res, df_case_studies, df_gain_case_studies, df_cost_case_studies, "EUR", df_currencies)