#include "Configuration.h"
#include "SerialMessage.hpp"
// =================== 配置参数 ===================
#define MAX_DEGREE 60
#define MAX_COEFFS ((MAX_DEGREE + 1) * (MAX_DEGREE + 2) / 2)

const double WMM_A = 6378.137;     // 地球长半轴 [km]
const double WMM_B = 6356.7523142; // 地球短半轴 [km]
const double WMM_RE = 6371.2;      // WMM 模型使用的参考球半径

// =================== 数据结构 ===================
typedef struct
{
    int n, m;
    double g, h;
    double g_dot, h_dot;
} GaussCoeff;

GaussCoeff *wmm_table = NULL;
int wmm_table_size = 0;
double wmm_epoch_year = 0.0;

// =================== COF 载入 ===================
int load_cof(const char *path)
{
    wmm_table = (GaussCoeff *)pvPortMalloc(MAX_COEFFS * sizeof(GaussCoeff));
    if (!wmm_table)
    {
        Serial0_Println("Malloc memory for wm_table failed!");
        return -2;
    }
    File file = SPIFFS.open(path, "r");
    if (!file || file.isDirectory())
    {
        Serial0_Println("Open Con file failed!");
        return -1;
    }

    char line[256];
    int first = 1;

    while (file.available())
    {
        size_t len = file.readBytesUntil('\n', line, sizeof(line) - 1);
        line[len] = '\0';

        if (line[0] == '#' || strlen(line) < 10)
            continue;

        char model[16];
        int n, m;
        double g, h, g_dot, h_dot;
        if (sscanf(line, "%15s %d %d %lf %lf %lf %lf", model, &n, &m, &g, &h, &g_dot, &h_dot) != 7)
            continue;
        if (n > MAX_DEGREE)
            continue;

        wmm_table[wmm_table_size++] = (GaussCoeff){n, m, g, h, g_dot, h_dot};

        if (first)
        {
            int year;
            sscanf(model, "WMMHR.%4d", &year);
            wmm_epoch_year = (double)year;
            first = 0;
        }
    }
    Serial0_Println("wmm_table load succeed!");
    file.close();
    return 0;
}

// =================== 获取系数 ===================
double get_coeff(int n, int m, double dt, int type)
{
    for (int i = 0; i < wmm_table_size; i++)
    {
        if (wmm_table[i].n == n && wmm_table[i].m == m)
        {
            return (type == 0)
                       ? wmm_table[i].g + wmm_table[i].g_dot * dt
                       : wmm_table[i].h + wmm_table[i].h_dot * dt;
        }
        // vTaskDelay(pdMS_TO_TICKS(1));
    }
    return 0.0;
}

// =================== 勒让德函数 ===================
void compute_legendre(double theta, double P[MAX_DEGREE + 1][MAX_DEGREE + 1], double dP[MAX_DEGREE + 1][MAX_DEGREE + 1])
{
    for (int n = 0; n <= MAX_DEGREE; ++n)
    {

        for (int m = 0; m <= MAX_DEGREE; ++m)
        {
            P[n][m] = dP[n][m] = 0.0;
            // vTaskDelay(pdMS_TO_TICKS(1));
        }
    }

    P[0][0] = 1.0;
    double ct = cos(theta), st = sin(theta);

    for (int n = 1; n <= MAX_DEGREE; ++n)
    {
        for (int m = 0; m <= n; ++m)
        {
            if (n == m)
            {
                P[n][m] = st * P[n - 1][m - 1];
                dP[n][m] = st * dP[n - 1][m - 1] + ct * P[n - 1][m - 1];
            }
            else if (n == 1 || m == n - 1)
            {
                P[n][m] = ct * P[n - 1][m];
                dP[n][m] = ct * dP[n - 1][m] - st * P[n - 1][m];
            }
            else
            {
                double k = ((n - 1) * (n - 1) - m * m) / (double)((2 * n - 1) * (2 * n - 3));
                P[n][m] = ct * P[n - 1][m] - k * P[n - 2][m];
                dP[n][m] = ct * dP[n - 1][m] - st * P[n - 1][m] - k * dP[n - 2][m];
            }
            // vTaskDelay(pdMS_TO_TICKS(1));
        }
    }
}

// =================== 主计算函数 ===================
double calc_declination(double lat_deg, double lon_deg, double alt_km, double year)
{
    double phi = lat_deg * DEG_TO_RAD;
    double lambda = lon_deg * DEG_TO_RAD;

    double sin_phi = sin(phi), cos_phi = cos(phi);
    double a2 = WMM_A * WMM_A, b2 = WMM_B * WMM_B;
    double rho = sqrt(a2 * cos_phi * cos_phi + b2 * sin_phi * sin_phi);
    double r = sqrt(alt_km * alt_km + 2 * alt_km * rho + (a2 * b2) / (rho * rho));
    double theta = acos((alt_km + (b2 / rho)) * sin_phi / r);

    double P[MAX_DEGREE + 1][MAX_DEGREE + 1], dP[MAX_DEGREE + 1][MAX_DEGREE + 1];
    compute_legendre(theta, P, dP);

    double cos_m[MAX_DEGREE + 1], sin_m[MAX_DEGREE + 1];
    cos_m[0] = 1.0;
    sin_m[0] = 0.0;
    cos_m[1] = cos(lambda);
    sin_m[1] = sin(lambda);
    for (int m = 2; m <= MAX_DEGREE; ++m)
    {
        cos_m[m] = cos_m[m - 1] * cos_m[1] - sin_m[m - 1] * sin_m[1];
        sin_m[m] = sin_m[m - 1] * cos_m[1] + cos_m[m - 1] * sin_m[1];
    }

    double Br = 0, Bt = 0, Bp = 0;
    double dt = year - wmm_epoch_year;

    for (int n = 1; n <= MAX_DEGREE; ++n)
    {
        double rn = pow(WMM_RE / r, n + 2);
        for (int m = 0; m <= n; ++m)
        {
            double g = get_coeff(n, m, dt, 0);
            double h = get_coeff(n, m, dt, 1);
            double factor = rn * (g * cos_m[m] + h * sin_m[m]);
            Br += (n + 1) * factor * P[n][m];
            Bt -= factor * dP[n][m];
            if (m != 0)
            {
                Bp += rn * m * (g * sin_m[m] - h * cos_m[m]) * P[n][m] / sin(theta);
            }
            // vTaskDelay(pdMS_TO_TICKS(1));
        }
    }

    double Bx = -Bt;
    double By = Bp;
    if (wmm_table != NULL)
    {
        vPortFree(wmm_table);
        wmm_table = NULL;
    }

    return atan2(By, Bx) * RAD_TO_DEG;
}

struct MagDecReq
{
    double lat;
    double lon;
    double alt;
    double year;
};
struct MagDecRes
{
    double decl;
};
static QueueHandle_t queueHandle_MagDecReq = NULL;
static QueueHandle_t queueHandle_MagDecRes = NULL;

static TaskHandle_t xTaskHandle_MagneticDeclination = NULL;

static SemaphoreHandle_t semphr_MagneticDeclination_Mutex = NULL;
static bool is_MagDecRunning = false;

bool MagneticDeclinationSendReq(const MagDecReq *mdreq);
void MagneticDeclinationReceiveRes(MagDecRes *mdres);

bool Is_MagneticDeclination_running();
void set_MagneticDeclination_running(bool state);
void task_MagneticDeclination(void *parameters);

// configMAX_PRIORITIES - 3
void task_MagneticDeclination(void *parameters)
{
    queueHandle_MagDecReq = xQueueCreate(1, sizeof(MagDecReq));
    queueHandle_MagDecRes = xQueueCreate(1, sizeof(MagDecRes));
    xTaskHandle_MagneticDeclination = xTaskGetCurrentTaskHandle();
    MagDecReq mdreq;
    MagDecRes mdres;
    for (;;)
    {
        if (xQueueReceive(queueHandle_MagDecReq, &mdreq, portMAX_DELAY) == pdPASS)
        {
            set_MagneticDeclination_running(true);
            double decl = calc_declination(mdreq.lat, mdreq.lon, mdreq.alt, mdreq.year);
            mdres.decl = decl;
        }
        xQueueSend(queueHandle_MagDecRes, &mdres, (TickType_t)0);
        set_MagneticDeclination_running(false);
    }
}

bool Is_MagneticDeclination_running()
{
    bool result = false;
    if (xSemaphoreTake(semphr_MagneticDeclination_Mutex, portMAX_DELAY))
    {
        result = is_MagDecRunning;
        xSemaphoreGive(semphr_MagneticDeclination_Mutex);
    }
    return result;
}

void set_MagneticDeclination_running(bool state)
{
    if (xSemaphoreTake(semphr_MagneticDeclination_Mutex, portMAX_DELAY))
    {
        is_MagDecRunning = state;
        xSemaphoreGive(semphr_MagneticDeclination_Mutex);
    }
}

bool MagneticDeclinationSendReq(const MagDecReq *mdreq)
{

    if (Is_MagneticDeclination_running())
    {
        return false;
    }
    return xQueueSend(queueHandle_MagDecReq, mdreq, (TickType_t)0) == pdPASS;
}
void MagneticDeclinationReceiveRes(MagDecRes *mdres)
{
    if (xQueueReceive(queueHandle_MagDecRes, mdres, portMAX_DELAY) == pdPASS)
    {
        // We got a mdres
    }
}