from pandas import read_csv
from forex_python.converter import CurrencyRates
from json import dumps

# ref_currency is the short code for the currency used as reference (EUR, USD, etc.)
def balance_sheet(arr_sol_nb, df_case_studies, df_gain_case_studies, df_cost_case_studies, ref_currency, df_currencies):
    results = {}
    arr_eco_cost_per_sol = []
    arr_eco_gain_per_sol = []
    arr_energy_per_sol = []
    for i in range(0, len(arr_sol_nb)):
        nb_sol = arr_sol_nb[i]
        avg_cost = eco_cost_bal_sheet_sol(nb_sol, df_case_studies, df_cost_case_studies, ref_currency, df_currencies)
        if (avg_cost != -1):
            arr_eco_cost_per_sol.append(avg_cost)
        else:
            arr_eco_cost_per_sol.append(None)
        avg_gain = eco_gain_bal_sheet_sol(nb_sol, df_case_studies, df_gain_case_studies, ref_currency, df_currencies)
        if (avg_gain != -1):
            arr_eco_gain_per_sol.append(avg_gain)
        else:
            arr_eco_gain_per_sol.append(None)
    for i in range(0, len(arr_sol_nb)):
        data_sol = {
            "nb_sol": arr_sol_nb[i],
            "financial_cost" : arr_eco_cost_per_sol[i],
            "financial_gain" : arr_eco_gain_per_sol[i]
        }
        results.update(data_sol)
    return(dumps(results))
            

            


def eco_cost_bal_sheet_sol(nb_sol, df_case_studies, df_cost_case_studies, ref_currency, df_currencies):
    currencyRates = CurrencyRates()
    code_ref_currency = df_currencies.loc[df_currencies.shortmonnaie == ref_currency]
    # If the currency is not in the database, use euro by default
    if (code_ref_currency.empty):
        code_ref_currency = df_currencies.loc[df_currencies.shortmonnaie == "EUR"].values[0]
    else:
        code_ref_currency = code_ref_currency.nummonnaie.values[0]
    count = 0
    avg_cost = 0
    df_costs_one_sol = df_cost_case_studies.loc[df_cost_case_studies.codesolution == nb_sol]
    for j in range(0, len(df_costs_one_sol.index)):
        cost_currency = df_currencies.loc[df_currencies.nummonnaie == df_costs_one_sol.codemonnaiecoutrex.values[j]]
        if (df_costs_one_sol.reelcoutrex.values[j] != None):
            cost = df_costs_one_sol.reelcoutrex.values[j]
            count = count + 1
        # If real cost unavailable, use min and max cost instead if available
        elif (df_costs_one_sol.minicoutrex.values[j] != None and df_costs_one_sol.maxicoutrex.values[j] != None):
            cost = (df_costs_one_sol.minicoutrex.values[j] + df_costs_one_sol.maxicoutrex.values[j]) / 2
            count = count + 1
        # Use cost within case study if everything else is unavailable
        else:
            df_case_study = df_case_studies.loc[df_case_studies.numrex == df_costs_one_sol.coderex.values[j]]
            if (df_case_study.capexrex.values[0] != None):
                cost = df_case_study.capexrex.values[0]
                cost_currency = df_currencies.loc[df_currencies.nummonnaie == df_case_study.codemonnaie.values[0]]
                count = count + 1
            # If no cost registered, then ignore
            else:
                cost = 0
        if (cost_currency.nummonnaie.values[0] != code_ref_currency):
                rate = currencyRates.get_rate(ref_currency, cost_currency.shortmonnaie.values[0])
                ref_currency_cost = cost * rate
        else:
            ref_currency_cost = cost
        avg_cost = avg_cost + ref_currency_cost
    if (count > 0):
        avg_cost = avg_cost / count
    else:
        avg_cost = -1
    return(avg_cost)

def eco_gain_bal_sheet_sol(nb_sol, df_case_studies, df_gain_case_studies, ref_currency, df_currencies):
    currencyRates = CurrencyRates()
    code_ref_currency = df_currencies.loc[df_currencies.shortmonnaie == ref_currency]
    # If the currency is not in the database, use euro by default
    if (code_ref_currency.empty):
        code_ref_currency = df_currencies.loc[df_currencies.shortmonnaie == "EUR"].values[0]
    else:
        code_ref_currency = code_ref_currency.nummonnaie.values[0]
    count = 0
    avg_gain = 0
    df_gains_one_sol = df_gain_case_studies.loc[df_gain_case_studies.codesolution == nb_sol]
    for j in range(0, len(df_gains_one_sol)):
        gain_currency = df_currencies.loc[df_currencies.nummonnaie == df_gains_one_sol.codemonnaiegainrex.values[j]]
        if (df_gains_one_sol.gainfinanciergainrex.values[j] != None):
            gain = df_gains_one_sol.gainfinanciergainrex.values[j]
            count = count + 1
        # If unavailable, use gain within case study
        else:
            df_case_study = df_case_studies.loc[df_case_studies.numrex == df_gains_one_sol.coderex.values[j]]
            if (df_case_study.gainfinancierex.values[0] != None):
                gain = df_case_study.gainfinancierex.values[0]
                gain_currency = df_currencies.loc[df_currencies.nummonnaie == df_case_study.codemonnaie.values[0]]
                count = count + 1
            # If no gain registered, then ignore
            else:
                gain = 0
        if (gain_currency.nummonaie.values[0] != code_ref_currency):
            rate = currencyRates.get_rate(ref_currency, gain_currency.shortmonnaie.values[0])
            ref_currency_gain = gain * rate
        else:
            ref_currency_gain = gain
        avg_gain = avg_gain + ref_currency_gain
    if (count > 0):
        avg_gain = avg_gain / count
    else:
        avg_gain = -1
    return(avg_gain)


def energy_balance_sheet(nb_sol, df_case_studies, df_gain_case_studies, ref_currency, df_currencies):
    print()


case_studies_dataset = "./model/tblrex.csv"
gains_dataset = "./model/tblgainrex.csv"
costs_dataset = "./model/tblcostrex.csv"
currencies_dataset = "./modeltblmonnaie.csv"
df_case_studies = read_csv(case_studies_dataset, sep = ',', engine = 'python', quotechar = '"')
df_gain_case_studies = read_csv(gains_dataset, sep = ',', engine = 'python', quotechar = '"')
df_cost_case_studies = read_csv(costs_dataset, sep = ',', engine = 'python', quotechar = '"')
df_currencies = read_csv(currencies_dataset, sep = ',', engine = 'python', quotechar = '"')