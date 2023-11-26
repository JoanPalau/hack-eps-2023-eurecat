import os
import pandas as pd
import statsmodels.api as sm


def predict_next_48_hours(input_df):
    """
    Predicts the next 48 hours for each column in the given DataFrame.
    Parameters:
    input_df (DataFrame): A pandas DataFrame with a datetime index and columns to be forecasted.

    Returns:
    DataFrame: A DataFrame containing the 48-hour forecasts for each column.
    """

    # Ensure the index is in datetime format
    if not pd.api.types.is_datetime64_any_dtype(input_df.index):
        print("Index must be datetime type")
        return

    # Resample data to hourly averages
    hourly_data = input_df.resample('H').mean()

    # Dictionary to store forecasts
    forecasts = {}

    # Predicting for each column
    for column in hourly_data.columns:
        # Fit a SARIMAX model
        model = sm.tsa.statespace.SARIMAX(hourly_data[column],
                                          order=(1, 0, 1),
                                          seasonal_order=(1, 0, 1, 24)
                                          )  # Assuming daily seasonality
        results = model.fit(disp=False)

        # Forecasting the next 48 hours
        forecasts[column] = results.forecast(steps=48)  # 48 hours

    # Combining forecasts into a DataFrame
    forecast_df = pd.DataFrame(forecasts, index=pd.date_range(
        start=hourly_data.index[-1], periods=49, freq='H')[1:])

    return forecast_df

# Example usage (your colleague would replace 'your_dataframe' with their DataFrame)
# forecasted_values = predict_next_48_hours(your_dataframe)


df = pd.read_sql_table('data', os.environ.get('HACKEPS_DATABASE_URI'))
df = df.drop(columns=['device_id', 'id', 'extra_data'])
df_indexed = df.set_index('created_at').copy()
print(df_indexed)
test = predict_next_48_hours(df_indexed)
import pdb; pdb.set_trace()
