#include "ClusterSpec.h"
#include <Timer.h>
#include <Config/Config.h>
#include <Database/DatabaseEnv.h>
#include <Policies/SingletonImp.h>
#include <ObjectMgr.h>

INSTANTIATE_SINGLETON_1( ClusterBasic );
volatile bool ClusterBasic::m_stopEvent = false;
uint8 ClusterBasic::m_ExitCode = SHUTDOWN_EXIT_CODE;

ClusterBasic::ClusterBasic()
{
}

ClusterBasic::~ClusterBasic()
{
}

void ClusterBasic::run()
{
	// Init
	///- Init new SQL thread for the world database
    WorldDatabase.ThreadStart();                                // let thread do safe mySQL requests (one connection call enough)
	uint32 realCurrTime = 0;
    uint32 realPrevTime = getMSTime();
	uint32 prevSleepTime = 0;                               // used for balanced full tick time length near WORLD_SLEEP_CONST

	while(!MustStop())
	{
		// Updates
		realCurrTime = getMSTime();
        uint32 diff = getMSTimeDiff(realPrevTime,realCurrTime);

		// Main update there if need
		realPrevTime = realCurrTime;
		if (diff <=  CLUSTER_SLEEP_CONST+prevSleepTime)
        {
            prevSleepTime = CLUSTER_SLEEP_CONST+prevSleepTime-diff;
            ACE_Based::Thread::Sleep(prevSleepTime);
        }
        else
            prevSleepTime = 0;
	}
	
	// Exit Cleanup there

	///- End the database thread
    WorldDatabase.ThreadEnd();                                  // free mySQL thread resources
}

void ClusterBasic::SetInitialSettings()
{
	///- Time server startup
    uint32 uStartTime = getMSTime();
	
	///- Initialize config settings
    LoadConfigSettings();

	///- Load the DBC files
    sLog.outString("Initialize data stores...");
	LoadDBCStores("./");
    DetectDBCLang();

	// For other clusters, modify loaded tables there

	sLog.outString( "CLUSTER: ClusterBasic initialized" );

	uint32 uStartInterval = getMSTimeDiff(uStartTime, getMSTime());
	sLog.outBasic( "CLUSTER STARTUP TIME: %i minutes %i seconds", uStartInterval / 60000, (uStartInterval % 60000) / 1000 );
}

void ClusterBasic::DetectDBCLang()
{
    uint32 m_lang_confid = sConfig.GetIntDefault("DBC.Locale", 255);

    if(m_lang_confid != 255 && m_lang_confid >= MAX_LOCALE)
    {
        sLog.outError("Incorrect DBC.Locale! Must be >= 0 and < %d (set to 0)",MAX_LOCALE);
        m_lang_confid = LOCALE_enUS;
    }

    ChrRacesEntry const* race = sChrRacesStore.LookupEntry(1);

    std::string availableLocalsStr;

    int default_locale = MAX_LOCALE;
    for (int i = MAX_LOCALE-1; i >= 0; --i)
    {
        if ( strlen(race->name[i]) > 0)                     // check by race names
        {
            default_locale = i;
            m_availableDbcLocaleMask |= (1 << i);
            availableLocalsStr += localeNames[i];
            availableLocalsStr += " ";
        }
    }

    if( default_locale != m_lang_confid && m_lang_confid < MAX_LOCALE &&
        (m_availableDbcLocaleMask & (1 << m_lang_confid)) )
    {
        default_locale = m_lang_confid;
    }

    if(default_locale >= MAX_LOCALE)
    {
        sLog.outError("Unable to determine your DBC Locale! (corrupt DBC?)");
        exit(1);
    }

    m_defaultDbcLocale = LocaleConstant(default_locale);

    sLog.outString("Using %s DBC Locale as default. All available DBC locales: %s",localeNames[m_defaultDbcLocale],availableLocalsStr.empty() ? "<none>" : availableLocalsStr.c_str());
    sLog.outString();
}

void ClusterBasic::LoadConfigSettings()
{
}

void ClusterBasic::Wait()
{
	while(!MustStop())
	{
		ACE_Based::Thread::Sleep(1000);
	}
}