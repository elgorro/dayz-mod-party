class SchanaPartyBasicMapAPI {
    static const string GROUP_ID_LOCAL = "schanamodparty_local";
    static const string GROUP_ID_REMOTE = "schanamodparty_remote";
    static const string GROUP_ID_MEMBERS = "schanamodparty_members";
    static const string DISPLAY_NAME_LOCAL = "MY PARTY MARKERS";
    static const string DISPLAY_NAME_REMOTE = "PARTY MEMBER MARKERS";
    static const string DISPLAY_NAME_MEMBERS = "PARTY MEMBERS";

    private static ref SchanaPartyBasicMapAPI api;

    static SchanaPartyBasicMapAPI Get () {
        if (!api) {
            api = new ref SchanaPartyBasicMapAPI;
            if (GetGame ().IsClient ()) {
                api.ClientInit ();
            } else {
                api.ServerInit ();
            }
        }
        return api;
    }

#ifdef BASICMAP
    private ref BasicMapController controller;
#endif

    private void SchanaPartyBasicMapAPI () { }

    void ~SchanaPartyBasicMapAPI () {
#ifdef BASICMAP
        if (GetGame ().IsClient ()) {
            GetGame ().GetCallQueue (CALL_CATEGORY_SYSTEM).Remove (this.SendMarkers);
        }
#endif
    }

    void ClientInit () {
        GetRPCManager ().AddRPC ("SchanaModParty", "ClientRegisterBasicMapMarkersRPC", this, SingleplayerExecutionType.Both);
        GetRPCManager ().AddRPC ("SchanaModParty", "ClientAddBasicMapMarkerRPC", this, SingleplayerExecutionType.Both);
        GetRPCManager ().AddRPC ("SchanaModParty", "ClientRemoveBasicMapMarkerRPC", this, SingleplayerExecutionType.Both);
#ifdef BASICMAP
        SchanaPartyUtils.LogMessage ("BasicMap Client Init");
        controller = BasicMap ();
        // the order groups are registered is the same as the order they appear in BasicMap
        BasicMapGroupMetaData localMeta = new BasicMapGroupMetaData (GROUP_ID_LOCAL, DISPLAY_NAME_LOCAL, true);
        BasicMapGroupMetaData remoteMeta = new BasicMapGroupMetaData (GROUP_ID_REMOTE, DISPLAY_NAME_REMOTE, false);
        BasicMapGroupMetaData memberMeta = new BasicMapGroupMetaData (GROUP_ID_MEMBERS, DISPLAY_NAME_MEMBERS, false);
        controller.RegisterGroup (GROUP_ID_LOCAL, localMeta, new SchanaPartyLocalMarkerFactory ());
        controller.RegisterGroup (GROUP_ID_REMOTE, remoteMeta, new SchanaPartyRemoteMarkerFactory ());
        controller.RegisterGroup (GROUP_ID_MEMBERS, memberMeta, NULL);

        GetGame ().GetCallQueue (CALL_CATEGORY_SYSTEM).CallLater (this.SendMarkers, 30000, true);
#endif
    }

    void ServerInit () {
        SchanaPartyUtils.LogMessage ("BasicMap Server Init");
        GetRPCManager ().AddRPC ("SchanaModParty", "ServerRegisterBasicMapMarkersRPC", this, SingleplayerExecutionType.Both);
        GetRPCManager ().AddRPC ("SchanaModParty", "ServerAddBasicMapMarkerRPC", this, SingleplayerExecutionType.Both);
        GetRPCManager ().AddRPC ("SchanaModParty", "ServerRemoveBasicMapMarkerRPC", this, SingleplayerExecutionType.Both);
    }

#ifdef BASICMAP

    void ClientRegisterBasicMapMarkersRPC (CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target) {
#ifdef BASICMAP
        Param1<ref array<ref BasicMapMarker>> data;
        if (!ctx.Read (data))
            return;

        auto markers = data.param1;
        int i;

        for (i = 0; i < markers.Count (); ++i) {
            ClientAddBasicMapMarker (markers.Get (i).GetName (), markers.Get (i).GetPosition ());
        }
#endif
    }

    void ClientAddBasicMapMarkerRPC (CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target) {
#ifdef BASICMAP
        Param2<string, vector> data;
        if (!ctx.Read (data))
            return;

        ClientAddBasicMapMarker (data.param1, data.param2);
#endif
    }

    void ClientRemoveBasicMapMarkerRPC (CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target) {
#ifdef BASICMAP
		
        Param1<vector> data;
        if (!ctx.Read (data))
            return;

        ClientRemoveBasicMapMarker (data.param1);
		
#endif
    }


    void ServerRegisterBasicMapMarkersRPC (CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target) {
#ifdef BASICMAP
        Param1<ref array<ref BasicMapMarker>> data;
        if (!sender || !ctx.Read (data))
            return;
        string id = sender.GetId ();

        ServerRegisterBasicMapMarkers (id, data);
#endif
    }


    void ServerAddBasicMapMarkerRPC (CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target) {
#ifdef BASICMAP
        Param2<string, vector> data;
        if (!sender || !ctx.Read (data))
            return;

        string id = sender.GetId ();
        auto manager = GetSchanaPartyManagerServer ();
		if (!manager)
			return;
			
        auto parties = manager.GetParties ();
		if (!parties)
			return;
		
		auto theParty = manager.GetPartyPlayers (id);
		if (!theParty)
			return;
		
        foreach (auto player : theParty) {
            DayZPlayer ply = DayZPlayer.Cast (player);
			if (ply && ply.GetIdentity () && ply.IsAlive ()){
				GetRPCManager ().SendRPC ("SchanaModParty", "ClientAddBasicMapMarkerRPC", data, false, ply.GetIdentity ());
			}
        }
#endif
    }

    void ServerRemoveBasicMapMarkerRPC (CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target) {
#ifdef BASICMAP
        Param1<vector> data;
        if (!ctx.Read (data))
            return;

        string id = sender.GetId ();
        auto manager = GetSchanaPartyManagerServer ();

        auto parties = manager.GetParties ();
		if (!parties)
			return;
			
		auto theParty = manager.GetPartyPlayers (id);
		if (!theParty)
			return;
		
        foreach (auto player : theParty) {
            DayZPlayer ply = DayZPlayer.Cast (player);
            if (ply && ply.GetIdentity () && ply.IsAlive ()) {
                GetRPCManager ().SendRPC ("SchanaModParty", "ClientRemoveBasicMapMarkerRPC", data, false, ply.GetIdentity ());
            }
        }
#endif

    }
	
#ifdef BASICMAP

    void ServerRegisterBasicMapMarkers (string id, Param1<ref array<ref BasicMapMarker>> data) {
        auto manager = GetSchanaPartyManagerServer ();
        ref array<DayZPlayer> players = manager.GetPartyPlayers (id)
        if (players) {
            foreach (auto player : players) {
				DayZPlayer ply = DayZPlayer.Cast (player);
				if (ply && ply.GetIdentity () && ply.IsAlive ()){
                    GetRPCManager ().SendRPC ("SchanaModParty", "ClientRegisterBasicMapMarkersRPC", data, false, ply.GetIdentity ());
                }
            }
        }
    }
	
    void ClientAddBasicMapMarker (string name, vector position) {
        AddOrUpdateMarker (GROUP_ID_REMOTE, name, position);
    }

    void ClientRemoveBasicMapMarker (vector position) {
        RemoveMarker (position);
    }
	
    void SendMarkers () {
        ref array<ref BasicMapMarker> markers = controller.GetMarkers (GROUP_ID_LOCAL);
        auto data = new Param1<ref array<ref BasicMapMarker>> (markers);
        GetRPCManager ().SendRPC ("SchanaModParty", "ServerRegisterBasicMapMarkersRPC", data);
    }

    void SendCreatedMarker (ref BasicMapMarker marker) {
        auto data = new Param2<string, vector> (marker.GetName (), marker.GetPosition ());
        GetRPCManager ().SendRPC ("SchanaModParty", "ServerAddBasicMapMarkerRPC", data);
    }

    void SendDeletedMarker (ref BasicMapMarker marker) {
        auto data = new Param1<vector> (marker.GetPosition ());
        GetRPCManager ().SendRPC ("SchanaModParty", "ServerRemoveBasicMapMarkerRPC", data);
    }

    void AddMarker (string group, string name, vector position) {
        controller.CreateMarker (group, name, position, true);
    }

    void AddOrUpdateMarker (string group, string name, vector position) {
        float distance = 0.1;
        BasicMapMarker marker = controller.GetMarkerByVector (position, distance, true);
        if (marker) {
            marker.Name = name;
        } else {
            AddMarker (group, name, position);
        }
    }

    void RemoveMarker (vector position) {
        float distance = 0.1;
        controller.RemoveMarkerByVector (position, distance, true);
    }
	
#endif
}