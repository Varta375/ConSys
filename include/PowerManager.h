#ifndef PowerManager_H
#define PowerManager_H

#include <windows.h>
#include <guiddef.h>
#include <powersetting.h>
#include <powrprof.h>


/**
 * @class PowerManager
 * @brief Класс для настройки электропитания ноутбука
 * @details Данные получены с помощью консольной команды powercfg -list.
 *			Позволяет переключаться между режимами "Экономия энергии", "Сбалансированный" и "Высокая производительность"
 *			Использует Windows API функции PowerSetActiveScheme
 * @see https://learn.microsoft.com/ru-ru/windows/win32/api/powersetting/nf-powersetting-powersetactivescheme
 */
class PowerManager {
public:

	/**
	 * @brief Устанавливает режим электропитания "Экономия энергии"
	 * @details Программа устанавливает стандартную схему электропитания Windows. Нацелена на экономию энергии путем понижения энергопотребления CPU и GPU
	 * @return true в случае, если режим смог примениться, false - при неудаче
	 * @throw std::exception Ошибки логируются в std::cerr, а в качестве результата возвращается false
	 */
	bool SetEcoMode();

	/**
	 * @brief Устанавливает режим электропитания "Сбалансированный"
	 * @details Программа устанавливает стандартную схему электропитания Windows. Нацелена на сбалансированную производительность системы путем динамического изменения энергопотребления CPU и GPU
	 * @return true в случае, если режим смог примениться, false - при неудаче
	 * @throw std::exception Ошибки логируются в std::cerr, а в качестве результата возвращается false
	 */
	bool SetBalanceMode();

	/**
	 * @brief Устанавливает режим электропитания "Высокая производительность"
	 * @details Программа устанавливает стандартную схему электропитания Windows. Нацелена на максимальную производительность системы
	 * @return true в случае, если режим смог примениться, false - при неудаче
	 * @throw std::exception Ошибки логируются в std::cerr, а в качестве результата возвращается false
	 */
	bool SetPerformanceMode();

private:
	static constexpr GUID ECO_GUID = { 0xa1841308 , 0x3541, 0x4fab, {0xbc, 0x81, 0xf7, 0x15, 0x56, 0xf2, 0x0b, 0x4a} }; ///< GUID для режима "Экономии энергии"
	static constexpr GUID BALANCED_GUID = { 0x381b4222, 0xf694, 0x41f0,{ 0x96, 0x85, 0xff, 0x5b, 0xb2, 0x60, 0xdf, 0x2e } }; ///< GUID для режима "Сбалансированный"
	static constexpr GUID PERFORMANCE_GUID = { 0x8c5e7fda, 0xe8bf, 0x4a96, {0x9a, 0x85, 0xa6, 0xe2, 0x3a, 0x8c, 0x63, 0x5c} }; ///< GUID для режима "Высокая производительность"
};

#endif