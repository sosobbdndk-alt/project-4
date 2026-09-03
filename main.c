#include <stdio.h>
#include <stdint.h>
#include <string.h>

#define SOURCE_COUNT 4U
#define SYSTEM_COUNT 6U
#define NAME_LEN     16U

typedef struct {
    char     name[NAME_LEN];
    uint16_t generated; /* Watts */
    uint8_t  online;    /* 1 = active, 0 = disabled */
} Source_t;

typedef struct {
    char     name[NAME_LEN];
    uint16_t load;      /* Watts */
    uint8_t  priority;  /* 1 = Critical (Highest), 6 = Low */
    uint8_t  powered;   /* 1 = on, 0 = off (shed) */
} Subsystem_t;

static Source_t    sources[SOURCE_COUNT];
static Subsystem_t systems[SYSTEM_COUNT];

/* Function Prototypes */
static void     initGrid(void);
static uint32_t totalGeneration(void);
static uint32_t totalActiveLoad(void);
static void     showGrid(void);
static void     toggleSource(void);
static void     setSystemLoad(void);
static void     autoShed(void);
static void     restoreSystems(void);
static void     powerReport(void);

static int readInt(int *out)
{
    char buf[64];
    if (fgets(buf, (int)sizeof(buf), stdin) == NULL) {
        return 0;
    }
    return sscanf(buf, "%d", out) == 1;
}

static void initGrid(void)
{
    const char *srcNames[SOURCE_COUNT] = { "Solar Array A", "Solar Array B", "Main Reactor", "Aux Battery" };
    const uint16_t srcPwr[SOURCE_COUNT] = { 1200U, 1200U, 3000U, 800U };

    for (uint8_t i = 0U; i < SOURCE_COUNT; i++) {
        strncpy(sources[i].name, srcNames[i], NAME_LEN - 1U);
        sources[i].name[NAME_LEN - 1U] = '\0';
        sources[i].generated = srcPwr[i];
        sources[i].online = 1U;
    }

    const char *sysNames[SYSTEM_COUNT] = {
        "Life Support", "Shields", "Comms Array", "Thrusters", "Thermal Control", "Science Lab"
    };
    const uint16_t sysLoads[SYSTEM_COUNT] = { 1500U, 1800U, 600U, 1000U, 700U, 900U };
    const uint8_t  sysPrio[SYSTEM_COUNT]  = { 1U, 2U, 3U, 4U, 5U, 6U };

    for (uint8_t i = 0U; i < SYSTEM_COUNT; i++) {
        strncpy(systems[i].name, sysNames[i], NAME_LEN - 1U);
        systems[i].name[NAME_LEN - 1U] = '\0';
        systems[i].load = sysLoads[i];
        systems[i].priority = sysPrio[i];
        systems[i].powered = 1U;
    }
}

static uint32_t totalGeneration(void)
{
    uint32_t total = 0U;
    for (uint8_t i = 0U; i < SOURCE_COUNT; i++) {
        if (sources[i].online) {
            total += sources[i].generated;
        }
    }
    return total;
}

static uint32_t totalActiveLoad(void)
{
    uint32_t total = 0U;
    for (uint8_t i = 0U; i < SYSTEM_COUNT; i++) {
        if (systems[i].powered) {
            total += systems[i].load;
        }
    }
    return total;
}

static void showGrid(void)
{
    printf("\n--- Power Sources ---\n");
    for (uint8_t i = 0U; i < SOURCE_COUNT; i++) {
        printf("[%u] %-15s : %4u W [%s]\n",
               i, sources[i].name, sources[i].generated,
               sources[i].online ? "ONLINE" : "OFFLINE");
    }

    printf("\n--- Subsystems ---\n");
    for (uint8_t i = 0U; i < SYSTEM_COUNT; i++) {
        printf("[%u] %-16s : Load %4u W (Prio %u) [%s]\n",
               i, systems[i].name, systems[i].load, systems[i].priority,
               systems[i].powered ? "POWERED" : "SHED");
    }

    uint32_t gen = totalGeneration();
    uint32_t load = totalActiveLoad();
    printf("\nTotal Generation: %u W | Total Load: %u W (Balance: %d W)\n",
           gen, load, (int32_t)gen - (int32_t)load);
}

static void toggleSource(void)
{
    int id = 0;
    printf("Power Source ID to toggle (0..%u): ", SOURCE_COUNT - 1U);
    if (!readInt(&id) || id < 0 || id >= (int)SOURCE_COUNT) {
        printf("Invalid Source ID!\n");
        return;
    }
    sources[id].online = !sources[id].online;
    printf("Source %s is now %s.\n", sources[id].name, sources[id].online ? "ONLINE" : "OFFLINE");
    autoShed();
}

static void setSystemLoad(void)
{
    int id = 0;
    int val = 0;
    printf("Subsystem ID (0..%u): ", SYSTEM_COUNT - 1U);
    if (!readInt(&id) || id < 0 || id >= (int)SYSTEM_COUNT) {
        printf("Invalid System ID!\n");
        return;
    }
    printf("New load (0..5000 W): ");
    if (!readInt(&val) || val < 0 || val > 5000) {
        printf("Invalid load range!\n");
        return;
    }
    systems[id].load = (uint16_t)val;
    printf("Load updated for %s.\n", systems[id].name);
    autoShed();
}

static void autoShed(void)
{
    uint32_t gen = totalGeneration();
    uint32_t load = totalActiveLoad();

    if (load <= gen) {
        return;
    }

    printf("\n[ALERT] Grid Overload! Initiating emergency load shedding...\n");
    for (uint8_t p = 6U; p >= 1U; p--) {
        for (uint8_t i = 0U; i < SYSTEM_COUNT; i++) {
            if (systems[i].priority == p && systems[i].powered) {
                systems[i].powered = 0U;
                printf("  -> Disconnected: %s (%u W)\n", systems[i].name, systems[i].load);
                load = totalActiveLoad();
                if (load <= gen) {
                    printf("Grid stabilized.\n");
                    return;
                }
            }
        }
    }
}

static void restoreSystems(void)
{
    uint32_t gen = totalGeneration();
    printf("\nAttempting to reconnect systems...\n");

    for (uint8_t p = 1U; p <= 6U; p++) {
        for (uint8_t i = 0U; i < SYSTEM_COUNT; i++) {
            if (systems[i].priority == p && !systems[i].powered) {
                if (totalActiveLoad() + systems[i].load <= gen) {
                    systems[i].powered = 1U;
                    printf("  -> Reconnected: %s (%u W)\n", systems[i].name, systems[i].load);
                }
            }
        }
    }
    printf("Restoration cycle completed.\n");
}

static void powerReport(void)
{
    uint32_t gen = totalGeneration();
    uint32_t load = totalActiveLoad();
    uint8_t shedCount = 0U;

    for (uint8_t i = 0U; i < SYSTEM_COUNT; i++) {
        if (!systems[i].powered) {
            shedCount++;
        }
    }

    printf("\n============= POWER GRID REPORT =============\n");
    printf("Total Available Power : %u W\n", gen);
    printf("Active Power Draw     : %u W\n", load);
    printf("Net Power Margin      : %d W\n", (int32_t)gen - (int32_t)load);
    printf("Status                : %s\n", shedCount > 0U ? "DEFICIT (Load Shed Active)" : "NOMINAL");
    printf("Shed Subsystems Count : %u of %u\n", shedCount, SYSTEM_COUNT);
    printf("=============================================\n");
}

int main(void)
{
    initGrid();
    int opt = -1;

    do {
        printf("\n1.Status 2.ToggleSource 3.ChangeLoad 4.AutoShed 5.Restore 6.Report 0.Exit > ");
        if (!readInt(&opt)) {
            printf("Invalid input!\n");
            continue;
        }

        switch (opt) {
            case 1: showGrid(); break;
            case 2: toggleSource(); break;
            case 3: setSystemLoad(); break;
            case 4: autoShed(); break;
            case 5: restoreSystems(); break;
            case 6: powerReport(); break;
            case 0: printf("Shutting down grid monitoring...\n"); break;
            default: printf("Unknown option!\n"); break;
        }
    } while (opt != 0);

    return 0;
}