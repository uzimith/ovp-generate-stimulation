#include "ovpCBoxAlgorithmGenerateStimulation.h"

using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBE::Plugins;

using namespace OpenViBEPlugins;
using namespace OpenViBEPlugins::Tools;

boolean CBoxAlgorithmGenerateStimulation::initialize(void)
{
	m_oInput0Decoder.initialize(*this, 0);
	m_oOutput0Encoder.initialize(*this, 0);
	
	m_ui64StartStimulation = FSettingValueAutoCast(*this->getBoxAlgorithmContext(), 0);
	m_ui64TriggerStimulation = FSettingValueAutoCast(*this->getBoxAlgorithmContext(), 1);
	m_sSequence = FSettingValueAutoCast(*this->getBoxAlgorithmContext(), 2);
	m_ui64Index = 0;

	// If you need to retrieve setting values, use the FSettingValueAutoCast function.
	// For example :
	// - CString setting at index 0 in the setting list :
	// CString l_sSettingValue = FSettingValueAutoCast(*this->getBoxAlgorithmContext(), 0);
	// - unsigned int64 setting at index 1 in the setting list :
	// uint64 l_ui64SettingValue = FSettingValueAutoCast(*this->getBoxAlgorithmContext(), 1);
	// - float64 setting at index 2 in the setting list :
	// float64 l_f64SettingValue = FSettingValueAutoCast(*this->getBoxAlgorithmContext(), 2);
	// ...

	return true;
}
/*******************************************************************************/

boolean CBoxAlgorithmGenerateStimulation::uninitialize(void)
{
	m_oInput0Decoder.uninitialize();
	m_oOutput0Encoder.uninitialize();

	return true;
}
/*******************************************************************************/

/*
boolean CBoxAlgorithmGenerateStimulation::processClock(IMessageClock& rMessageClock)
{
	// some pre-processing code if needed...

	// ready to process !
	getBoxAlgorithmContext()->markAlgorithmAsReadyToProcess();

	return true;
}
*/
/*******************************************************************************/

/*
uint64 CBoxAlgorithmGenerateStimulation::getClockFrequency(void)
{
	// Note that the time is coded on a 64 bits unsigned integer, fixed decimal point (32:32)
	return 1LL<<32; // the box clock frequency
}
*/
/*******************************************************************************/


boolean CBoxAlgorithmGenerateStimulation::processInput(uint32 ui32InputIndex)
{
	// some pre-processing code if needed...

	// ready to process !
	getBoxAlgorithmContext()->markAlgorithmAsReadyToProcess();

	return true;
}

/*******************************************************************************/

/*
boolean CBoxAlgorithmGenerateStimulation::processMessage(const IMessageWithData& msg, uint32 inputIndex)
{
	//If you know what the message should contain, you can directly access the values by using 
	//getters of the message class with known keys. Otherwise, you can loop over the contents to discover the keys.
	
	//You can get the first CString key of the message by calling this function
	//const CString *l_sKey = msg.getFirstCStringToken();
	//You can then go through all the keys by calling
	// getNextCStringToken(previousKey)
	//The function will return NULL when no more keys are found
#if 0
	while(l_sKey!=NULL)
	{
		l_sKey = msg.getNextCStringToken(*l_sKey);
		//and access the content with
		CString* l_sContent;
		boolean ok = msg.getValueCString(l_sKey, &l_sContent);
		//if ok is false, the retrieval was not successful
		//the message will be deleted when the function goes out of scope, store the value if you wish to use it later
	}
	
	//Same thing for the other types
	const CString *l_sMatrixKey = msg.getFirstIMatrixToken();
	while(l_sMatrixKey!=NULL)
	{
		l_sMatrixKey = msg.getNextIMatrixToken(*l_sMatrixKey);
		//and access the content with
		IMatrix* l_oContent;
		boolean ok = msg.getValueIMatrix(l_sMatrixKey, &l_oContent);
		//if ok is false, the retrieval was not successful
		//the message will be deleted when the function goes out of scope, store the value if you wish to use it later
		//for matrices, the copy is done that way
		//CMatrix * l_oLocalMatrix = new CMatrix();
		//OpenViBEToolkit::Tools::Matrix::copy(*l_oLocalMatrix, *l_oContent);
	}
#endif
	
	// Remember to return false in case the message was unexpected (user has made a wrong connection)	
	return true;
}
*/
/*******************************************************************************/

boolean CBoxAlgorithmGenerateStimulation::process(void)
{

	IBoxIO& l_rDynamicBoxContext=this->getDynamicBoxContext();
	IStimulationSet* l_pOutputStimulations = m_oOutput0Encoder.getInputStimulationSet();
	l_pOutputStimulations->clear();

	//iterate over all chunk on input 0
	for(uint32 i=0; i<l_rDynamicBoxContext.getInputChunkCount(0); i++)
	{
		// decode the chunk i
		m_oInput0Decoder.decode(i);
		if(m_oInput0Decoder.isHeaderReceived())
		{
			m_oOutput0Encoder.encodeHeader();
            l_rDynamicBoxContext.markOutputAsReadyToSend(0, l_rDynamicBoxContext.getInputChunkStartTime(0, i), l_rDynamicBoxContext.getInputChunkEndTime(0, i));
		}
		if(m_oInput0Decoder.isBufferReceived())
		{
            IStimulationSet* l_pStimulations = m_oInput0Decoder.getOutputStimulationSet();
            for(uint32 j=0; j<l_pStimulations->getStimulationCount(); j++)
            {
				if(l_pStimulations->getStimulationIdentifier(j) == m_ui64StartStimulation ||
					l_pStimulations->getStimulationIdentifier(j) == m_ui64TriggerStimulation)
				{

					OpenViBE::uint64  l_ui64Target = m_sSequence[m_ui64Index] - '0' + OVTK_StimulationId_Label_00;
					m_ui64Index = (m_ui64Index + 1) % m_sSequence.length();

					CString l_sStimulationName   = this->getTypeManager().getEnumerationEntryNameFromValue(OV_TypeId_Stimulation, l_ui64Target);
					this->getLogManager() << LogLevel_Info << "generate command :" << l_sStimulationName << "\n";

					uint64 l_ui64StimulationDate = this->getPlayerContext().getCurrentTime();

					l_pOutputStimulations->appendStimulation(l_ui64Target, l_ui64StimulationDate, 0);
				 
				}
				m_oOutput0Encoder.encodeBuffer();
				l_rDynamicBoxContext.markOutputAsReadyToSend(0, l_rDynamicBoxContext.getInputChunkStartTime(0, i), l_rDynamicBoxContext.getInputChunkEndTime(0, i));
			}
		}
		if(m_oInput0Decoder.isEndReceived())
		{
			m_oOutput0Encoder.encodeEnd();
            l_rDynamicBoxContext.markOutputAsReadyToSend(0, l_rDynamicBoxContext.getInputChunkStartTime(0, i), l_rDynamicBoxContext.getInputChunkEndTime(0, i));
		}
	}


	return true;
}