#include <unity.h>
#include "StateMachine.h"

// ========================================
// TESTS PARA STATEMACHINE
// ========================================

// Mock objects (simulaciones simples de hardware y sensores)
StateMachine stateMachine;

void setUp(void) {
    // Se ejecuta antes de cada test
}

void tearDown(void) {
    // Se ejecuta después de cada test
}

// ========================================
// TESTS DE CONFIGURACIÓN
// ========================================

void test_program_defaults_22() {
    ProgramConfig config;
    config.setDefaults(PROGRAM_22);

    TEST_ASSERT_EQUAL(PROGRAM_22, config.programNumber);
    TEST_ASSERT_EQUAL(1, config.totalProcesses);
    TEST_ASSERT_EQUAL(WATER_HOT, config.waterType[0]);
}

void test_program_defaults_23() {
    ProgramConfig config;
    config.setDefaults(PROGRAM_23);

    TEST_ASSERT_EQUAL(PROGRAM_23, config.programNumber);
    TEST_ASSERT_EQUAL(1, config.totalProcesses);
    TEST_ASSERT_EQUAL(WATER_COLD, config.waterType[0]);
}

void test_program_defaults_24() {
    ProgramConfig config;
    config.setDefaults(PROGRAM_24);

    TEST_ASSERT_EQUAL(PROGRAM_24, config.programNumber);
    TEST_ASSERT_EQUAL(4, config.totalProcesses);
}

// ========================================
// TESTS DE ESTADOS
// ========================================

void test_initial_state() {
    stateMachine.begin();
    TEST_ASSERT_EQUAL(STATE_INIT, stateMachine.getState());
}

void test_state_transition_to_welcome() {
    stateMachine.begin();
    stateMachine.update(); // INIT -> WELCOME
    TEST_ASSERT_EQUAL(STATE_WELCOME, stateMachine.getState());
}

void test_program_selection() {
    stateMachine.selectProgram(PROGRAM_22);
    ProgramConfig& config = stateMachine.getConfig();
    TEST_ASSERT_EQUAL(PROGRAM_22, config.programNumber);
}

// ========================================
// TESTS DE TIEMPO
// ========================================

void test_elapsed_time_tracking() {
    stateMachine.begin();
    delay(100);
    unsigned long elapsed = stateMachine.getPhaseElapsedTime();
    TEST_ASSERT_GREATER_OR_EQUAL(100, elapsed);
}

// ========================================
// MAIN DE TESTS
// ========================================

void setup() {
    delay(2000); // Esperar inicialización

    UNITY_BEGIN();

    // Tests de configuración
    RUN_TEST(test_program_defaults_22);
    RUN_TEST(test_program_defaults_23);
    RUN_TEST(test_program_defaults_24);

    // Tests de estados
    RUN_TEST(test_initial_state);
    RUN_TEST(test_state_transition_to_welcome);
    RUN_TEST(test_program_selection);

    // Tests de tiempo
    RUN_TEST(test_elapsed_time_tracking);

    UNITY_END();
}

void loop() {
    // Vacío - los tests solo corren una vez
}
