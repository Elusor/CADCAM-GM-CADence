#pragma once

struct LoopData
{
	bool m_qOnBorder = false;
	bool m_pOnBorder = false;
	bool m_qLooped = false;
	bool m_pLooped = false;

	bool GetIsLooped() {
		return m_qLooped || m_pLooped;
	}
};